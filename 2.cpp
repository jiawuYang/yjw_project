#include <iostream>

using namespace std;


class Buliding
{

    friend void bulidshow(Buliding buliding);
public:
    Buliding()
    {
        setting_room = "gg";
        bed_room = "pp";
    }

    string setting_room;
private:

    string bed_room;
};

void bulidshow(Buliding buliding)
{
    cout << "sdf " << buliding.setting_room << endl;
    cout << "sdf " << buliding.bed_room << endl;
}

void test01()
{
    Buliding buliding;
    bulidshow(buliding);
}

int main()
{
    test01();


    system("pause");
    return 0;
}