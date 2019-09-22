#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
using namespace std;

unsigned short index, cpu[65536];
class OptiCode{
public:
    enum typeComad{
        SHIFT,
        ADD,
        ZERO,
        OUT,
        IN,
        WHILE,
        END,
        Default
    };
    OptiCode(typeComad type, int countArg)
    {
        this->type = type;
        this->arg = countArg;
    }

    OptiCode(typeComad type)
    {
        this->type = type;
    }
    int arg = 1;
    typeComad type = Default; //тип операции
    OptiCode* clone(){
        return new OptiCode(type, arg);
    }
};

class Tokenizer{

    vector<OptiCode*> retValue;
public:
    vector<OptiCode*>& tokenize(vector<char>& code) {

        //Приходимся по всем символам
        for(unsigned int pos = 0; pos < code.size(); ++pos) {
            switch (code[pos]) {
            //Как и говорилось ранее, некоторые команды эквивалентны
            case '>': { retValue.emplace_back(new OptiCode(OptiCode::typeComad::SHIFT, +1)); break; }
            case '<': { retValue.emplace_back(new OptiCode(OptiCode::typeComad::SHIFT, -1)); break; }

            case '+': { retValue.emplace_back(new OptiCode(OptiCode::typeComad::ADD, +1)); break; }
            case '-': { retValue.emplace_back(new OptiCode(OptiCode::typeComad::ADD, -1)); break; }

            case '.': { retValue.emplace_back(new OptiCode(OptiCode::typeComad::OUT)); break; }
            case ',': { retValue.emplace_back(new OptiCode(OptiCode::typeComad::IN)); break; }
            case '[':
            {
                char next = code[pos + 1];

                //проверяем, является ли это обнулением ячейки ([+] или [-])
                if((next == '+' || next == '-') && code[pos + 2] == ']') {
                    retValue.emplace_back(new OptiCode(OptiCode::typeComad::ZERO));
                    pos += 2;
                } else
                    retValue.emplace_back(new OptiCode(OptiCode::typeComad::WHILE));
                break;
            }
            case ']': { retValue.emplace_back(new OptiCode(OptiCode::typeComad::END)); break; }
            default: break;
            }
        }

        return retValue;
    }
    void clearData()
    {
        for (auto it = retValue.begin() ; it != retValue.end(); ++it)
            if((*it) != nullptr)
                delete (*it);
    }
};

class Optimizer {


public:
    vector<OptiCode*> optimize(vector<char>& code) {
        return optimize(tokenizer.tokenize(code));
    }
    void clearData()
    {
        tokenizer.clearData();
        for (auto it = retValue.begin() ; it != retValue.end(); ++it)
            if((*it) != nullptr)
                delete (*it);
    }
private:
    vector<OptiCode*> retValue;
    Tokenizer tokenizer;
    vector<OptiCode*>& optimize(vector<OptiCode*>& tokens) {
        retValue.emplace_back(tokens.front()->clone());//это первая итерация, добавляем 1 элемент
        //Приходимся по всем командам начиная со 2
        for (unsigned int token = 1; token < tokens.size(); token++) {

            if(retValue.back()->type != tokens[token]->type)
            {
                if(retValue.back()->arg == 0) //если в результате сжатия команда "исчезла"
                    retValue.pop_back(); //то просто убираем ее

                retValue.emplace_back(tokens[token]->clone()); //добавляем текущую команду
                continue;
            }
            //сюда мы попадет при условии, если команда дальше повторяется
            //мы просто дополняем текущую команду вместо добавления новой
            retValue.back()->arg += tokens[token]->arg;

        }
        return retValue;
    }
};

class InterpreterBf
{
private:
    Optimizer optimizer;
public:
    void interpret(vector<char>& code){
        interpret(optimizer.optimize(code));
    }
    void clearData()
    {
        optimizer.clearData();
    }
private:
    void interpret(vector<OptiCode*> compresCode)
    {
        int brc = 0; // счетчик незакрытых скобок.
        for( size_t numElement = 0; numElement < compresCode.size(); ++numElement )
        {
            switch (compresCode[numElement]->type){
            case OptiCode::SHIFT: { index+= compresCode[numElement]->arg; break; }
            case OptiCode::ADD: { cpu[index]+= compresCode[numElement]->arg; break;}
            case OptiCode::OUT:
            {
//                int16_t value = 0;
//                value = (static_cast<int16_t>(cpu[index - 1] << 8)) + static_cast<int16_t>(cpu[index]);
                for(int i = 0; i < compresCode[numElement]->arg; i++)
                    cout << (char)cpu[index];
                break;
            }
            case OptiCode::IN: { cin >> cpu[index]; break;}
            case OptiCode::ZERO: { cpu[index] = 0; break;}
            case OptiCode::WHILE:
            {
                if(!cpu[index]) //Если значение по текущему адресу ноль.
                {
                    ++brc; //Инкрементируем счетчик скобок.
                    while(brc) // Пока есть не закрытые скобки.
                    {
                        ++numElement; //К следующему символу.
                        if (compresCode[numElement]->type == OptiCode::WHILE) ++brc; //Открываем скобку.
                        if (compresCode[numElement]->type == OptiCode::END) --brc; //Закрываем скобку.
                    }
                }else continue; //Если не ноль берем следующий символ.
                break;
            }
            case OptiCode::END:
            {
                if(!cpu[index]) //Если значение по адресу ноль.
                {
                    continue; //Переходим к следующему символу.
                }
                else //Если не ноль.
                {
                    if(compresCode[numElement]->type == OptiCode::END) brc++; //Если скобка закрывающаяся  инкрементируем счетчик скобок.
                    while(brc) //Пока есть незакрытые скобки.
                    {
                        --numElement; // Смотрим предыдущий символ.
                        if (compresCode[numElement]->type == OptiCode::WHILE) brc--; //Если скобка открытая декрементируем счетчик.
                        if (compresCode[numElement]->type == OptiCode::END) brc++; //Если закрытая инкрементируем счетчик.
                    }
                    --numElement; //Смотрим предыдущий символ.
                }
                break;
            }
            default: break;
            }
        }
    }
};

void interpreterOriginBF(vector<char>& inputData)
{

    unsigned short j = 0; //Регистр текущего адреса памяти интерпретатора.
    int brc = 0; // счетчик незакрытых скобок.
    for( size_t i = 0; i < inputData.size(); ++i )
    {
        if(inputData[i] == '>') j++;
        if(inputData[i] == '<') j--;
        if(inputData[i] == '+') cpu[j]++;
        if(inputData[i] == '-') cpu[j]--;
        if(inputData[i] == '.') cout << (char)cpu[j];
        if(inputData[i] == ',') cin >> cpu[j];
        if(inputData[i] == '[')
        {
            if(!cpu[j]) //Если значение по текущему адресу ноль.
            {
                ++brc; //Инкрементируем счетчик скобок.
                while(brc) // Пока есть не закрытые скобки.
                {
                    ++i; //К следующему символу.
                    if (inputData[i] == '[') ++brc; //Открываем скобку.
                    if (inputData[i] == ']') --brc; //Закрываем скобку.
                }
            }else continue; //Если не ноль берем следующий символ.
        }
        else if(inputData[i] == ']') //Если скобка заркывающаяся.
        {
            if(!cpu[j]) //Если значение по адресу ноль.
            {
                continue; //Переходим к следующему символу.
            }
            else //Если не ноль.
            {
                if(inputData[i] == ']') brc++; //Если скобка закрывающаяся  инкрементируем счетчик скобок.
                while(brc) //Пока есть незакрытые скобки.
                {
                    --i; // Смотрим предыдущий символ.
                    if(inputData[i] == '[') brc--; //Если скобка открытая декрементируем счетчик.
                    if(inputData[i] == ']') brc++; //Если закрытая инкрементируем счетчик.
                }
                --i; //Смотрим предыдущий символ.
            }
        }
    }
}
int main()
{
    vector<char> inputData;
    ifstream file("test.txt");
    if(!file.is_open())
        cout << "file doesn't exist";

    char c;
    while (file.get(c))
      inputData.push_back(c);

    auto start_time =  clock();
//    interpreterOriginBF(inputData);
    InterpreterBf token;
    token.interpret(inputData);
    auto end_time = clock();
    auto search_time = end_time - start_time;

    cout << "program execution time is " << search_time;
    token.clearData();
    return 0;
}



