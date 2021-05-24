#include "vector.h"
#include "bigint.h"

#include <random>
#include <sstream>
#include <tuple>
#include <array>
#include <vector>
#include <cassert>

void TestVector() {
    {
        TVector myVec;
        std::vector<uint32_t> stlVec;
        for (uint32_t i = 0; i < 1000; ++i) {
            myVec.push_back(i);
            stlVec.push_back(i);
        }
        assert(myVec.size() == stlVec.size());
        assert(myVec.capacity() == stlVec.capacity());
        assert(std::equal(myVec.begin(), myVec.end(), stlVec.begin()));
    }
    {
        TVector myVec;
        std::vector<uint32_t> stlVec;
        myVec.reserve(15);
        stlVec.reserve(15);
        assert(myVec.capacity() == stlVec.capacity());

        for (uint32_t i = 0; i < 16; ++i) {
            myVec.push_back(i);
            stlVec.push_back(i);
        }
        assert(myVec.back() == 15);

        myVec.pop_back();
        assert(myVec.capacity() > 15);
        assert(myVec.size() == 15);
        assert(myVec.capacity() == stlVec.capacity());
    }
    {
        TVector myVec;
        assert(myVec.empty());

        std::vector<uint32_t> stlVec;
        assert(myVec.capacity() == stlVec.capacity());
        assert(myVec.size() == stlVec.size());
        assert(myVec.begin() == stlVec.data());
    }
    {
        TVector myVec1 = { 1, 2, 3, 4, 5 };
        TVector myVec2 = { 6, 7, 8, 9, 0 };
        assert(!std::equal(myVec1.begin(), myVec1.end(), myVec2.begin()));

        TVector myVec3 = std::move(myVec1);
        assert(!myVec1.size() && !myVec1.capacity());
        std::vector<uint32_t> tmp = { 1, 2, 3, 4, 5 };
        assert(myVec3.size() == tmp.size());
        assert(std::equal(myVec3.rbegin(), myVec3.rend(), tmp.rbegin()));

        myVec1 = std::move(myVec2);
        assert(!myVec2.size() && !myVec2.capacity());
        tmp = { 6, 7, 8, 9, 0 };
        assert(myVec1.size() == tmp.size());
        assert(std::equal(myVec1.begin(), myVec1.end(), tmp.begin()));
    }
    std::cerr << "TestVector is OK" << std::endl;
}

std::string runCommand(const std::string &command) {
    std::ostringstream request;
    request << "echo '" << command << "' | ./script.py";
    FILE *ifs = popen(request.str().c_str(), "r");
    char buffer[512];
    if (fscanf(ifs, "%s", buffer) == 1) {
        fclose(ifs);
        return buffer;
    }
    fclose(ifs);
    return "";
}

void checkOperation(const std::vector<std::pair<std::string, std::string>> &operands, char op) {
    for (const auto &[lhs, rhs] : operands) {
        std::pair<TBigInt, TBigInt> numbers = std::make_pair(TBigInt(lhs), TBigInt(rhs));
        TBigInt product;
        switch (op) {
        case '+':
            product = numbers.first + numbers.second;
            break;
        case '-':
            product = numbers.first - numbers.second;
            break;
        case '*':
            product = numbers.first * numbers.second;
            break;
        default:
            break;
        }
        std::ostringstream os;
        os << product;
        assert(os.str() == runCommand(lhs + " " + op + " " + rhs));
    }
}

std::string generateRandomNumber(size_t length) {
    static std::mt19937 rand_gen(time(0));
    std::string res;
    res.reserve(length + 1);
    res.push_back(rand_gen() & 1 ? '+' : '-');
    for (size_t i = 0; i < length; ++i) {
        res.push_back('0' + (rand_gen() % 10));
    }
    return res;
}

std::string_view removePlusPrefix(std::string_view str) {
    if (str.size() && str.front() == '+') {
        str.remove_prefix(1);
    }
    return str;
}

void TestBigInt() {
    // {                                       // comparing output with python script
    //     constexpr size_t operandsQuantity = 16;
    //     constexpr size_t randomNumberLength = 128;
    //     std::vector<std::pair<std::string, std::string>> terms(operandsQuantity);
    //     for (auto &[lhs, rhs] : terms) {
    //         lhs = generateRandomNumber(randomNumberLength);
    //         rhs = generateRandomNumber(randomNumberLength);
    //     }
    //     checkOperation(terms, '+');
    //     checkOperation(terms, '-');
    //     checkOperation(terms, '*');
    // }
    {                                       // io operators
        std::array<std::istringstream, 4> inputs = {
            std::istringstream("3901381239408349345771209432747289178329484533713"),
            std::istringstream("+4093458238234294"),
            std::istringstream("-15"),
            std::istringstream("0")
        };
        TBigInt n;
        for (std::istringstream &input : inputs) {
            std::ostringstream output;
            input >> n;
            output << n;
            assert(removePlusPrefix(input.str()) == output.str());
        }
    }
    {                                       // unary minus
        std::vector<std::string> numbers = {
            "-34573947923842911239897459",
            "-1",
            "-0"
        };
        std::vector<std::string_view> numbersPos;
        numbersPos.reserve(numbers.size());
        for (const std::string &it : numbers) {
            numbersPos.push_back(std::string_view(&*it.begin() + 1, it.size() - 1));
        }
        for (size_t i = 0; i < numbers.size(); ++i) {
            TBigInt neg(numbers[i]), pos(numbersPos[i]);
            assert(neg == -pos);
        }
    }
    {                                       // int32_t support
        TBigInt n = 17;
        assert(n == 17);
        assert(n + 100 == 117);
        assert(n - 100 == -83);
        assert(n * 100 == 1700);
    }
    {                                       // test from example
        std::ostringstream os;
        TBigInt a = 1;
        TBigInt b("123456789012345678901234567890");
        TBigInt c = a * b + 2;
        TBigInt d;
        d = std::move(c);
        a = d + b;
        os << a;
        assert(os.str() == "246913578024691357802469135782");
    }
    {                                       // operator+
        std::vector<std::tuple<std::string, std::string, std::string>> expressions = {
            std::make_tuple("946120931239582323409234985283472319871231095034", "043873897487123123873456", "946120931239582323409235029157369806994354968490"),
            std::make_tuple("-458976452934282431092350394123", "32434983794539845837329453749", "-426541469139742585255020940374"),
            std::make_tuple("234897234", "-123812398129954", "-123812163232720"),
            std::make_tuple("999999999999999999999999999999999999999999999999", "0", "999999999999999999999999999999999999999999999999"),
            std::make_tuple("999999999999999999999999999999999999999999999999", "1", "1000000000000000000000000000000000000000000000000"),
            std::make_tuple("999999999999999999999999999999999999999999999999", "-1", "999999999999999999999999999999999999999999999998"),
            std::make_tuple("9999", "1", "10000")
        };
        for (const auto &[lhs, rhs, result] : expressions) {
            std::ostringstream os;
            os << TBigInt(lhs) + TBigInt(rhs);
            assert(os.str() == result);
        }
    }
    {                                       // operator-
        std::vector<std::tuple<std::string, std::string, std::string>> expressions = {
            std::make_tuple("946120931239582323409234985283472319871231095034", "043873897487123123873456", "946120931239582323409234941409574832748107221578"),
            std::make_tuple("-458976452934282431092350394123", "32434983794539845837329453749", "-491411436728822276929679847872"),
            std::make_tuple("234897234", "-123812398129954", "123812633027188"),
            std::make_tuple("0", "999999999999999999999999999999999999999999999999", "-999999999999999999999999999999999999999999999999"),
            std::make_tuple("999999999999999999999999999999999999999999999999", "-1", "1000000000000000000000000000000000000000000000000"),
            std::make_tuple("999999999999999999999999999999999999999999999999", "1", "999999999999999999999999999999999999999999999998"),
            std::make_tuple("9999", "-1", "10000")
        };
        for (const auto &[lhs, rhs, result] : expressions) {
            std::ostringstream os;
            os << TBigInt(lhs) - TBigInt(rhs);
            assert(os.str() == result);
        }
    }
    {                                       // operator*
        std::vector<std::tuple<std::string, std::string, std::string>> expressions = {
            std::make_tuple("946120931239582323409234985283472319871231095034", "043873897487123123873456", "41510012747626900767766071304626751312262276372208574991117898526017504"),
            std::make_tuple("-458976452934282431092350394123", "32434983794539845837329453749", "-14886893812998830927048117068878383963022819818516449917127"),
            std::make_tuple("234897234", "-123812398129954", "-29083189855632967147236"),
            std::make_tuple("0", "999999999999999999999999999999999999999999999999", "0"),
            std::make_tuple("-1", "999999999999999999999999999999999999999999999999", "-999999999999999999999999999999999999999999999999"),
            std::make_tuple("999999999999999999999999999999999999999999999999", "1", "999999999999999999999999999999999999999999999999"),
            std::make_tuple("1000", "10", "10000")
        };
        for (const auto &[lhs, rhs, result] : expressions) {
            std::ostringstream os;
            os << TBigInt(lhs) * TBigInt(rhs);
            assert(os.str() == result);
        }
    }
    {                                       // operator<=>
        TBigInt lhs("2304923123095045623042375938439");
        TBigInt rhs(lhs);

        assert(lhs == rhs);
        assert(lhs <= rhs);
        assert(lhs >= rhs);

        rhs = TBigInt("2304923123095045623042375938440");

        assert(lhs != rhs);
        assert(lhs < rhs);
        assert(rhs > lhs);

        rhs = TBigInt("-2304923123095045623042375938439");

        assert(lhs != rhs);
        assert(rhs < lhs);
        assert(lhs > rhs);

        lhs = TBigInt(0);
        rhs = TBigInt("-0");

        assert(lhs == rhs);
    }
    std::cerr << "TestBigInt is OK" << std::endl;
}

int main() {
    TestVector();
    TestBigInt();
    return 0;
}
