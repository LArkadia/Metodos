#include <string>
#include <iostream>
namespace PSR{
    class Ecuation{
            private:
            float x1,x2,result;
            uint equality;
            public:
            Ecuation(float x,float x2,float result,char e);
            float get_x1();
            float get_x2();
            float get_result();
            char get_operator();
            void Print();
        };

    Ecuation* Parser(std::string ecuation){
        float x1,x2,result;
        char e;
        sscanf(ecuation.c_str(),"%f %f %c %f",&x1,&x2,&e,&result);
        return new Ecuation(x1,x2,result,e);

    }

    Ecuation::Ecuation(float x, float x2,float result, char e){
        this->x1 = x;
        this->x2 = x2;
        this->result = result;
        equality = e;
    }

    float Ecuation::get_x1()
    {
        return x1;
    }

    float Ecuation::get_x2()
    {
        return x2;
    }

    float Ecuation::get_result()
    {
        return result;
    }

    char Ecuation::get_operator()
    {
        return equality;
    }

    void Ecuation::Print(){
        printf("%.2fx1 + %.2fx2 %c %.2f\n",x1,x2,equality,result);
    }
}