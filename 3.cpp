#include <iostream>
#include <string>

using namespace std;

class Buliding
{
    friend class goodgay;
public:
    Buliding()
    {
        settingroom = "gg";
        bedroom = "pp";
    }

public:
    string settingroom;
private:
    string bedroom;

};

// class Buliding;
class goodgay
{
public:
    goodgay()
    {
        buliding = new Buliding;
    }
    void visit()
    {
        cout << "dfg = " << buliding->settingroom << endl;
        cout << "dfg = " << buliding->bedroom << endl;       
    }
    Buliding *buliding;
};

// Buliding::Buliding()
// {
//     settingroom = "gg";
//     bedroom = "pp";
// }

// goodgay::goodgay()
// {
//     buliding = new Buliding;
// }

// void goodgay::visit()
// {
//     cout << "dfg = " << buliding->settingroom << endl;
//     cout << "dfg = " << buliding->bedroom << endl;
// }

void test01()
{
    goodgay p;
    p.visit();
}


int main()
{
    test01();


    system("pause");
    return 0;
}






