class A {
    void hi() {}
    int a;
};

class B : public A {
    virtual void bye() {}
};

int main() {
    B b;
}
