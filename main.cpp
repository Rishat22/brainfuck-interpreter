#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
using namespace std;

unsigned short index, cpu[ 65536 ];
class OptiCode{
public:
    enum TypeComad{
        SHIFT,
        ADD,
        ZERO,
        OUT,
        IN,
        WHILE,
        END,
        DEFAULT
    };
    OptiCode( TypeComad type, int count_arg )
    {
        this->mType = type;
        this->mArg = count_arg;
    }

    OptiCode( TypeComad type )
    {
        this->mType = type;
    }
    int mArg = 1;
    TypeComad mType = DEFAULT; //тип операции
    OptiCode* clone(){
        return new OptiCode( mType, mArg );
    }
};

class Tokenizer{

    vector< OptiCode* > mRetValue;
public:
    vector< OptiCode* >& Tokenize( vector< char >& code ) {

        //Приходимся по всем символам
        for( unsigned int pos = 0; pos < code.size(); ++pos ) {
            switch ( code[ pos ] ) {
            //Как и говорилось ранее, некоторые команды эквивалентны
            case '>': { mRetValue.emplace_back( new OptiCode( OptiCode::TypeComad::SHIFT, +1 ) ); break; }
            case '<': { mRetValue.emplace_back( new OptiCode( OptiCode::TypeComad::SHIFT, -1 ) ); break; }

            case '+': { mRetValue.emplace_back( new OptiCode( OptiCode::TypeComad::ADD, +1 ) ); break; }
            case '-': { mRetValue.emplace_back( new OptiCode( OptiCode::TypeComad::ADD, -1 ) ); break; }

            case '.': { mRetValue.emplace_back( new OptiCode( OptiCode::TypeComad::OUT ) ); break; }
            case ',': { mRetValue.emplace_back( new OptiCode( OptiCode::TypeComad::IN ) ); break; }
            case '[':
            {
                char next = code[ pos + 1 ];

                //проверяем, является ли это обнулением ячейки ( [+] или [-] )
                if( ( next == '+' || next == '-' ) && code[ pos + 2 ] == ']' ) {
                    mRetValue.emplace_back( new OptiCode( OptiCode::TypeComad::ZERO ) );
                    pos += 2;
                } else
                    mRetValue.emplace_back( new OptiCode( OptiCode::TypeComad::WHILE ) );
                break;
            }
            case ']': { mRetValue.emplace_back( new OptiCode( OptiCode::TypeComad::END ) ); break; }
            default: break;
            }
        }

        return mRetValue;
    }
    void ClearData()
    {
        for (auto it = mRetValue.begin() ; it != mRetValue.end(); ++it)
            if((*it) != nullptr)
                delete (*it);
    }
};

class Optimizer {


public:
    vector< OptiCode* > Optimize( vector< char >& code ) {
        return Optimize( mTokenizer.Tokenize( code ) );
    }
    void ClearData()
    {
        mTokenizer.ClearData();
        for (auto it = mRetValue.begin() ; it != mRetValue.end(); ++it)
            if((*it) != nullptr)
                delete (*it);
    }
private:
    vector< OptiCode* > mRetValue;
    Tokenizer mTokenizer;
    vector< OptiCode* >& Optimize(vector< OptiCode* >& tokens) {
        mRetValue.emplace_back( tokens.front()->clone() );//это первая итерация, добавляем 1 элемент
        //Приходимся по всем командам начиная со 2
        for ( unsigned int token = 1; token < tokens.size(); token++ ) {

            if( mRetValue.back()->mType != tokens[ token ]->mType )
            {
                if( mRetValue.back()->mArg == 0 ) //если в результате сжатия команда "исчезла"
                    mRetValue.pop_back(); //то просто убираем ее

                mRetValue.emplace_back( tokens[ token ]->clone() ); //добавляем текущую команду
                continue;
            }
            //сюда мы попадет при условии, если команда дальше повторяется
            //мы просто дополняем текущую команду вместо добавления новой
            mRetValue.back()->mArg += tokens[ token ]->mArg;

        }
        return mRetValue;
    }
};

class InterpreterBf
{
private:
    Optimizer Optimizer;
public:
    void Interpret( vector< char >& code ){
        Interpret( Optimizer.Optimize( code ) );
    }
    void ClearData()
    {
        Optimizer.ClearData();
    }
private:
    void Interpret(vector<OptiCode*> compres_code)
    {
        int brc = 0; // счетчик незакрытых скобок.
        for( size_t pos = 0; pos < compres_code.size(); ++pos )
        {
            switch (compres_code[ pos ]->mType){
            case OptiCode::SHIFT: { index += compres_code[ pos ]->mArg; break; }
            case OptiCode::ADD: { cpu [index ] += compres_code[ pos ]->mArg; break;}
            case OptiCode::OUT:
            {
//                int16_t value = 0;
//                value = (static_cast<int16_t>(cpu[index - 1] << 8)) + static_cast<int16_t>(cpu[index]);
                for(int i = 0; i < compres_code[pos ]->mArg; i++)
                    cout << ( char )cpu[ index ];
                break;
            }
            case OptiCode::IN: { cin >> cpu[ index ]; break;}
            case OptiCode::ZERO: { cpu[ index ] = 0; break;}
            case OptiCode::WHILE:
            {
                if(!cpu[ index ]) //Если значение по текущему адресу ноль.
                {
                    ++brc; //Инкрементируем счетчик скобок.
                    while(brc) // Пока есть не закрытые скобки.
                    {
                        ++pos; //К следующему символу.
                        if ( compres_code[ pos ]->mType == OptiCode::WHILE ) ++brc; //Открываем скобку.
                        if ( compres_code[ pos ]->mType == OptiCode::END ) --brc; //Закрываем скобку.
                    }
                } else continue; //Если не ноль берем следующий символ.
                break;
            }
            case OptiCode::END:
            {
                if(!cpu[ index ]) //Если значение по адресу ноль.
                {
                    continue; //Переходим к следующему символу.
                }
                else //Если не ноль.
                {
                    if( compres_code[ pos ]->mType == OptiCode::END ) brc++; //Если скобка закрывающаяся  инкрементируем счетчик скобок.
                    while( brc ) //Пока есть незакрытые скобки.
                    {
                        --pos; // Смотрим предыдущий символ.
                        if ( compres_code[ pos ]->mType == OptiCode::WHILE ) brc--; //Если скобка открытая декрементируем счетчик.
                        if ( compres_code[ pos ]->mType == OptiCode::END ) brc++; //Если закрытая инкрементируем счетчик.
                    }
                    --pos; //Смотрим предыдущий символ.
                }
                break;
            }
            default: break;
            }
        }
    }
};

void InterpreterOriginBF( vector< char >& input_data)
{

    unsigned short j = 0; //Регистр текущего адреса памяти интерпретатора.
    int brc = 0; // счетчик незакрытых скобок.
    for( size_t index_data = 0; index_data < input_data.size(); ++index_data )
    {
        if( input_data[ index_data ] == '>' ) j++;
        if( input_data[ index_data ] == '<' ) j--;
        if( input_data[ index_data ] == '+' ) cpu[ j ]++;
        if( input_data[ index_data ] == '-' ) cpu[ j ]--;
        if( input_data[ index_data ] == '.' ) cout << ( char )cpu[ j ];
        if( input_data[ index_data ] == ',' ) cin >> cpu[ j ];
        if( input_data[ index_data ] == '[' )
        {
            if( !cpu[ j ] ) //Если значение по текущему адресу ноль.
            {
                ++brc; //Инкрементируем счетчик скобок.
                while( brc ) // Пока есть не закрытые скобки.
                {
                    ++index_data; //К следующему символу.
                    if ( input_data[ index_data ] == '[' ) ++brc; //Открываем скобку.
                    if ( input_data[ index_data ] == ']' ) --brc; //Закрываем скобку.
                }
            }else continue; //Если не ноль берем следующий символ.
        }
        else if( input_data[ index_data ] == ']' ) //Если скобка заркывающаяся.
        {
            if( !cpu[ j ] ) //Если значение по адресу ноль.
            {
                continue; //Переходим к следующему символу.
            }
            else //Если не ноль.
            {
                if( input_data[ index_data ] == ']' ) brc++; //Если скобка закрывающаяся  инкрементируем счетчик скобок.
                while( brc ) //Пока есть незакрытые скобки.
                {
                    --index_data; // Смотрим предыдущий символ.
                    if( input_data[ index_data ] == '[' ) brc--; //Если скобка открытая декрементируем счетчик.
                    if( input_data[ index_data ] == ']' ) brc++; //Если закрытая инкрементируем счетчик.
                }
                --index_data; //Смотрим предыдущий символ.
            }
        }
    }
}
int main()
{
    vector< char > input_data;
    ifstream file( "test.txt" );
    if( !file.is_open() )
        cout << "file doesn't exist";

    char c;
    while ( file.get( c ) )
      input_data.push_back( c );

    auto start_time =  clock();
//    interpreterOriginBF(inputData);
    InterpreterBf token;
    token.Interpret( input_data );
    auto end_time = clock();
    auto search_time = end_time - start_time;

    cout << "program execution time is " << search_time;
    token.ClearData();
    return 0;
}



