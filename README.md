**이 레포지토리는 C++11을 기반으로 합니다.**



# Move semantics

> 객체의 (동적으로 할당된 메모리 등)리소스를 다른 객체로 전송(이동)하는 것을 의미합니다.

C++03까지는 사용자 정의형 데이터 타입이나 포인터처럼 동적 메모리 영역을 이용할 때 복사 생성자와 복사 할당 연산자를 제공해야 깊은 복사(Deep copy)를 통해 정상적인 swap을 수행할 수 있었습니다.

하지만 객체가 무거워질수록 복사 비용도 커지면서 CPU, 메모리 등 자원을 낭비하게 되는 문제가 있고, C++11에서는 이 문제를 회피하기 위한 **Move semantics**를 도입했으며 rvalue reference를 사용하여 전체 복사로 인한 성능 저하를 회피할 수 있습니다.

함수(또는 멤버함수)의 매개 변수가 rvalue라는 것을 알고 있다면 rvalue 특성을 이용하여 매개 변수의 전체 내용을 복사하는 대신 내용만 이동(**move**)시킬 수 있는데(쉽게 말하면, **어떤 A라는 객체 내부에서 동적 할당된 메모리에 대한 소유권을 객체 B가 가질 수 있습니다.**) 이 과정에서 불필요한 동적 메모리 할당을 생략할 수 있는 것입니다.

**Move semantics**을 구현하기 위해 rvalue reference(&&)를 활용하게 되며, 함수 오버로딩을 통해 rvalue 인자(&&)를 감지하게 됩니다.





# rvalue reference

lvalue와 rvalue에 대한 간단한 예시입니다.

```c++
#include <iostream>
using namespace std;


void func(int& x) {
    cout << "lvalue ref : " << x << "\n";
}

void func(int&& x) {
    // 이 함수에서 매개변수가 x라는 이름을 가지고 있으므로 rvalue가 아니라 lvalue 입니다.
}

int main() {
    int k = 44;
    func(k); // lvalue ref called
    func(99); // rvalue ref called

    func(std::move(k)); // rvalue ref called
    func(static_cast<int&&>(k)); // rvalue ref called

    return 0;
}
```

```c++
// 임시 객체가 생성되는 경우 표현식은 rvalue 입니다.
int getValue() {
    int i = 22;
    return i; // i 는 lvalue이지만, getValue()가 반환하는 값의 형태는 rvalue 입니다.
}

int main() {
    ...
    int& lvalue_ref_value = getValue(); // Error: 비const 참조에 대한 초기값은 lvalue여야 합니다.
    const int &k_value = getValue(); // OK
    int&& rvalue_ref_value = getValue(); // OK
    
    return 0;
}
```



**참고 사항**

함수의 매개변수를 rvalue reference(&&) 형태로 받는다고 하여, 해당 변수가 rvalue인 것은 아닙니다.

**우측값 참조라고 정의된 것은 좌측값이 될 수 있고 우측값이 될 수도 있는데, (변수)이름이 있다면 좌측값이고 없다면 우측값입니다.**

따라서 아래의 예시에서 매개변수 `p_a`는 좌측값이며, `A v_a = p_a`에서 호출되는 생성자는 복사 생성자입니다.

```c++
void func(A&& p_a) {
    A v_a = p_a; // 좌측값이므로 A(const A&)가 호출됩니다.
}
```







# `std::move`

> **`std::move` 함수가 직접 이동을 수행하지는 않습니다. 매개변수로 받은 객체를 rvalue reference 형태로 변환하는 작업만 수행합니다.**

C++11은 Move semantics를 위해 `std::move` 함수를 지원합니다. `std::move` 함수는 인자로 받은 객체를 rvalue reference 형태로 변환하여 반환합니다.

`move`라는 이름만 보면, 함수를 통해서 무언가 이동시킬 것 같지만 실제로는 [`static_cast`](https://en.cppreference.com/w/cpp/language/static_cast)를 이용한 타입 변환(type conversion)만 수행합니다.



`std::move`를 사용한 코드를 다음과 같이 변경할 수 있으며 동일하게 동작합니다.

```c++
// use std::move
void move_swap(A& other) {
    A temp = std::move(other);
    other = std::move(*this);
    *this = std::move(temp);
}

// use type casting
void move_swap_(A& other) {
    A temp = static_cast<A&&>(other);
    other = static_cast<A&&>(*this);
    *this = static_cast<A&&>(temp);
}
```





# 복사 생성자, 복사 할당 연산자를 이용한 객체 swap

```c++
a.swap(b); // 여기서 swap은 클래스 A의 멤버 함수입니다.
---
void swap(A& other) {
    A temp = other; // or A temp(other);
    other = *this;
    *this = temp;
}
```

이 코드는 객체 a와 b의 내용을 swap하는 코드입니다. (타입 A의 형태를 알 수 없지만 기본적인 변수 swap이라고 생각합시다.) 일반적으로는 문제가 없어보이지만 객체 내부에 동적 메모리 영역을 활용하는 포인터 변수가 있다면 **얕은 복사**로 인하여 의도한 대로 복사가 이루어지지 않습니다.

더군다나 temp 객체의 라이프타임이 종료될 때 소멸자가 호출되면서 메모리를 영역을 해제하게 되는데, 이때 temp에서 해제한 메모리 영역이 원래는 other`(other는 객체 b)`의 포인터가 가리키고 있는 동적 메모리 영역이므로 b가 데이터(source)를 잃어버리는 문제가 발생합니다. 

따라서 아래의 코드와 같이 복사 생성자와 복사 할당 연산자를 정의해주어야 temp 객체의 메모리 영역을 새롭게 할당하여 swap을 수행할 수 있습니다.

```c++
#include <iostream>
#include <cstring>
#include <algorithm>
#include <utility>
using namespace std;


class A {
private:
    int* mData;
    int mLength;

public:

    // Default constructor
    A() : mData(nullptr), mLength(0) { }

    A(int pLength)
        :mLength(pLength), mData(new int[pLength]) {
        cout << "A(size_t). length = " << mLength << ".\n";
        for (int i = 0; i < pLength; ++i) {
            mData[i] = i;
        }
    }

    ~A() {
        cout << "~A(). length = " << mLength << ".";
        if (mData != nullptr) {
            cout << " Deleting resource.";
            delete[] mData;
            mData = nullptr;
        }
        cout << "\n";
    }

    void swap(A& other) {
        A temp(other);
        other = *this;
        *this = temp;
    }


    // 복사 생성자
    A(const A& other) : mLength(other.mLength), mData(new int[other.mLength]) {
        cout << "A(const A&). length = " << other.mLength << ". Copying resource." << "\n";
        copy(other.mData, other.mData + mLength, mData);
    }

    // 복사 할당 연산자
    A& operator=(const A& other) {
        cout << "operator=(A&). length = " << other.mLength << ". Copying resource." << "\n";
        if (this != &other) {
            delete[] mData;
            mLength = other.mLength;
            mData = new int[mLength];
            copy(other.mData, other.mData + mLength, mData);
        }
        return *this;
    }

    int* get_mData() const {
        return mData;
    }
};


int main() {

    A a(2), b(5);
    cout << a.get_mData() << " " << b.get_mData() << "\n";

    a.swap(b);

    cout << a.get_mData() << " " << b.get_mData() << "\n";
    return 0;
}
```

**result**

---

```text
A(size_t). length = 2.
A(size_t). length = 5.
00D70500 00D662A0
A(const A&). length = 5. Copying resource.
operator=(A&). length = 2. Copying resource.
operator=(A&). length = 5. Copying resource.
~A(). length = 5. Deleting resource.
00D662A0 00D70298
~A(). length = 2. Deleting resource.
~A(). length = 5. Deleting resource.
```



복사 생성자와 복사 할당 연산자를 정의하면 swap이 정상적으로 수행됩니다. 하지만 한 가지 눈여겨 봐야할 부분이 있는데, swap을 수행하기 전과 수행한 후의 메모리 주소가 달라졌다는 것입니다.

- swap 수행 전 메모리 주소 : **00D70500 00D662A0**

- swap 수행 후 메모리 주소 : **00D662A0 00D70298**

swap 수행 후 메모리 주소가 변경된 이유는 swap을 수행하는 과정에서 새로운 메모리를 할당하기 때문에 결과적으로 메모리 주소가 변경되기 때문입니다.

이것은 임시 객체에 대해 새롭게 메모리를 할당하고 최종적으로 다시 해제하는 과정을 수행하게 된다는 의미이며 객체가 무거워질수록 그만큼 복사 비용이 증가하기 때문에 수행 속도에 영향을 줍니다.





# 이동 생성자, 이동 할당 연산자를 이용한 객체 swap

rvalue reference를 활용하여 move semantics를 구현해보겠습니다.

아래는 `이동 생성자`와 `이동 할당 연산자`를 추가한 코드입니다.

```c++
a.move_swap(b); // 여기서 swap은 클래스 A의 멤버 함수입니다.
...
void move_swap(A& other) {
    A temp = std::move(other);
    other = std::move(*this);
    *this = std::move(temp);
}
    
// 이동 생성자
A(A&& other) noexcept {
    cout << "A(A&&). length = " << other.mLength << ". Moving resource.\n";

    mData = other.mData; // 포인터를 복사합니다.
    mLength = other.mLength;

    // 같은 주소에 대해 메모리 해제를 여러 번 하는 것을 방지하기 위해 nullptr로 설정합니다.
    other.mData = nullptr;
    other.mLength = 0;
}

// 이동 할당 연산자
A& operator=(A&& other) noexcept {
    cout << "operator=(A&&). length = " << other.mLength << "." << "\n";

    if (this != &other) { // 자기 자신을 대입하는 행위를 방지 ex) aObj = aObj;
        cout << "Address: " << mData << "\n";
        delete[] mData; // 할당될 개체의 메모리를 해제합니다.

        mData = other.mData; // 포인터를 복사합니다.
        mLength = other.mLength;

        // 같은 주소에 대해 메모리 해제를 여러 번 하는 것을 방지하기 위해 nullptr로 설정합니다.
        other.mData = nullptr;
        other.mLength = 0;
    }

    return *this; // 현재 개체에 대한 참조를 반환합니다.
}

...
```



`a.move_swap(b)`를 호출한 결과는 아래와 같습니다.

```text
A(size_t). length = 2.
A(size_t). length = 5.
0129FA80 012965A8
A(A&&). length = 5. Moving resource.
operator=(A&&). length = 2.
operator=(A&&). length = 5.
~A(). length = 0.
012965A8 0129FA80
~A(). length = 2. Deleting resource.
~A(). length = 5. Deleting resource.
```



- swap 수행 전 메모리 주소 : **0129FA80 012965A8**

- swap 수행 후 메모리 주소 : **012965A8 0129FA80**

사실 위의 swap 예시는 동적 메모리 할당과 메모리 주소에 대해 쉽게 나타내는 것이 중점을 둔 예시이므로, 굳이 rvalue reference를 활용한 이동 생성자와 이동 할당 연산자를 정의하지 않고도 똑같은 결과를 만들 수는 있습니다. 

아래에서 `vector`를 활용하여 이동 생성자, 이동 할당 연산자가 필요한 예시 코드를 살펴보겠습니다.





# 복사 생성자, 복사 할당 연산자만 사용했을 때

```c++
class A {
    // 클래스 정의는 동일함
};

int main() {
    vector<A> v;
    v.push_back(A(2)); // A(2)는 rvalue
    v.push_back(A(5)); // A(5)는 rvalue
    return 0;
}
```

**result**

```text
A(size_t). length = 2.
A(const A&). length = 2. Copying resource.
~A(). length = 2. Deleting resource.
A(size_t). length = 5.
A(const A&). length = 5. Copying resource.
A(const A&). length = 2. Copying resource.
~A(). length = 2. Deleting resource.
~A(). length = 5. Deleting resource.
~A(). length = 2. Deleting resource. (return 0; 에 의하여 프로그램이 종료되기 직전의 소멸자 호출)
~A(). length = 5. Deleting resource. (return 0; 에 의하여 프로그램이 종료되기 직전의 소멸자 호출)
```

먼저 `A(2)`가 **생성**되면 그 값을 벡터 v에 추가하기 위해 **A(2)의 내용을 그대로 복사**하게 됩니다.

생성되었던 `A(2)`는 rvalue 형태이므로 `v.push_back(A(2));` 식(expression)이 끝나면 더 이상 존재하지 않게 되고, 자연스럽게 소멸자가 호출되어 메모리를 해제합니다. 불필요한 메모리 할당과 해제 작업이 한 번씩 더 수행된 셈입니다.





# 이동 생성자, 이동 할당 연산자를 사용했을 때

바로 이전의 예시에서 처음부터 A(2)값을 벡터로 **이동**할 수 있다면 불필요한 메모리 할당과 해제가 발생하지 않게 되는데, 이 과정을 위해 이동 생성자와 이동 할당 생성자를 사용한 결과는 아래와 같습니다.

- 물론, push_back 함수가 rvalue reference 형태의 매개변수를 받도록 오버로딩 되어있기에 가능합니다.
- C++11부터 기본 STL에서 제공하는 함수도 rvalue reference를 활용한 Move semantics을 이용할 수 있도록 오버로딩 되어있습니다.

```c++
class A {
    // 클래스 정의는 동일함
};

int main() {
    vector<A> v;
    v.push_back(A(2)); // A(2)는 rvalue
    v.push_back(A(5)); // A(5)는 rvalue
    return 0;
}
```

**result**

```text
A(size_t). length = 2.
A(A&&). length = 2. Moving resource.
~A(). length = 0.
A(size_t). length = 5.
A(A&&). length = 5. Moving resource.
A(A&&). length = 2. Moving resource.
~A(). length = 0.
~A(). length = 0.
~A(). length = 2. Deleting resource. (return 0; 에 의하여 프로그램이 종료되기 직전의 소멸자 호출)
~A(). length = 5. Deleting resource. (return 0; 에 의하여 프로그램이 종료되기 직전의 소멸자 호출)
```



[위에서 정의한 코드](#이동-생성자-이동-할당-연산자를-이용한-객체-swap)에서는 객체의 포인터 변수가 rvalue 객체가 갖는 동적 메모리 주소를 가리키도록 하고, rvalue 객체가 소멸될 때 객체가 가리키고 있는 동일한 메모리 영역을 해제하지 않기 위해 `nullptr`로 설정해주었습니다.

이 과정을 통해 불필요한 복사 과정이 발생하지 않은 모습입니다.





## `noexcept` specifier

이동 생성자와 이동 할당 연산자의 정의된 부분을 보면 `noexcept`라는 키워드가 붙어있습니다.

```c++
A(A&& other) noexcept {
    ...
}

// 이동 할당 연산자
A& operator=(A&& other) noexcept {
    ...
}
```

noexcept는 해당함수가 exception을 발생하지 않는다고 명시하는 것이며 올바른 move semantics 작동을 위해서는 noexcept를 붙여주는 것이 일반적입니다.

1. move semantics는 보통 간단한 포인터 교환이나 리소스 핸들링으로 구현되므로 해당 함수에 exception이 발생하지 않게 구현합니다.
2. 함수에 `noexcept`을 붙입니다.

(noexcept를 사용할 때만 move가 수행되는 것은 아니지만 복사 생성자, 복사 할당 연산자, 이동 생성자, 이동 할당 연산자가 모두 정의되어 있다는 가정하에 std::vector 라이브러리는 move 함수가 noexcept 형태인지 검사하여 noexcept가 아닌 경우에는 move 대신에 copy를 이용합니다. 상황에 따라 noexcept를 붙이는게 필수일 수 있습니다.)





# 간단한 성능 비교

위에 예시로 등장했던 클래스 A를 rvalue 형태로 vector에 추가할 때 시간을 비교한 결과입니다.

Move semantics에서는 불필요한 메모리 할당과 해제가 발생하지 않기 때문에 좀 더 나은 속도 이점을 얻을 수 있습니다.

```c++
vector<A> v;
std::chrono::system_clock::time_point startTime = chrono::system_clock::now();
for (int i = 0; i < 1100000; ++i) {
    v.push_back(A(1000));
}
chrono::duration<double> sec = chrono::system_clock::now() - startTime;
cout << sec.count() << " sec\n";
```

|                         | 복사 생성자, 복사 할당 연산자 이용 | 이동 생성자, 이동 할당 연산자 이용 |
| :---------------------: | :--------------------------------: | :--------------------------------: |
| 실행 완료에 소비된 시간 |            9.32254 sec             |            1.73296 sec             |



