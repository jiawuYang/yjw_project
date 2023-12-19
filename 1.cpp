#include <iostream>

using namespace std;

class animal
{
public:
    virtual void speak()
    {
        cout << "animal speak" << endl;
    }
};

class cat: public animal
{
public:
    void speak()
    {
        cout << "cat speak" << endl;

    }
};

void dospeak(animal &animal)
{
    animal.speak();
}

void test01()
{
    cat p;
    dospeak(p);
}


int main()
{
    test01();

    system("pause");
    return 0;
}










