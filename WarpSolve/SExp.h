// -*- c++ -*-
// (C) 2013 David Richard Rush and Heureka Software
// http://cyber-rush.org.drr
// mailto:kumoyuki@gmail.com
//
// Permission to use is freely granted along with the inclusion of
// this copyright notice in all derivative works.
//
#pragma once

#include <memory>
#include <string>
#include <list>
#include <locale>
#include <stdio.h>
#include <vector>


#define BEGIN_HEUREKA namespace Heureka {
#define END_HEUREKA }

BEGIN_HEUREKA;
struct List;

struct Atom;
struct String;
struct Symbol;
struct Num;
struct Exact;
struct Inexact;
struct Fixnum;
struct Flonum;
struct Timestamp;


struct Datum
{
    struct Reader
    {
        struct Error
        {
            size_t Line;
            size_t Column;
            std::string Message;

            Error();
            Error(const Error& e);
            Error(size_t line, size_t col, const std::string msg);
            
            Error& operator= (const Error& e);
        };

        static std::unique_ptr<Reader> Read(const std::string& text);

        std::locale Locale;
        std::unique_ptr<Datum> Red;
        std::string::const_iterator Cursor;
        std::string::const_iterator End;
        size_t Line;
        size_t Column;
        std::list<Error> Errors;

        Reader(const std::string& s);

        void AddError(std::string m);

        void Deblank();

        bool Parse(void (Reader::* reader)());

        std::unique_ptr<Datum> ReadAtom();
        std::string ReadAtomicText();
        void ReadList();
        bool ReadNext();
        void ReadNumber();
        long long ReadNumberComponent();
        std::string ReadQuoted(char q);
        void ReadString();
        void ReadSymbol();
        void ReadTimestamp();
        std::pair<bool, long> ReadTimestampComponent(char delim, const char* what);
    };

    template <typename FROM> static std::unique_ptr<Datum> AsDatum(const FROM from);
    static std::unique_ptr<Datum> Read(const std::string& text);

    mutable std::string Text;

    virtual ~Datum();

    std::string AsString() const;

    virtual std::string BuildString() const =0;
    
    virtual Datum* DeepCopy() const =0;

    virtual bool IsAtom() const;
    virtual bool IsList() const;
    virtual bool IsNumber() const;
    virtual bool IsString() const;
    virtual bool IsTimestamp() const;
};


template <> std::unique_ptr<Datum> Datum::AsDatum(const double from);
template <> std::unique_ptr<Datum> Datum::AsDatum(const long from);
template <> std::unique_ptr<Datum> Datum::AsDatum(const std::string from);
template <> std::unique_ptr<Datum> Datum::AsDatum(FILE* from);


struct List : public Datum
{
    std::vector<Datum*> Data;

    ~List();

    std::vector<Datum*>::const_iterator begin() const;
    std::string BuildString() const;

    Datum* DeepCopy() const;

    std::vector<Datum*>::const_iterator end() const;

    bool IsList() const;

    size_t Length() const;

    Datum* Raw(size_t n) const;
    const Datum& Ref(size_t n) const;

    void Set(size_t at, Datum& d);
};


struct Atom : public Datum
{
    bool IsAtom() const;
};


struct Num : public Atom
{
    virtual long double Fraction() const =0;

    virtual bool IsExact() const;
    virtual bool IsInexact() const;
    bool IsNumber() const;

    virtual long long Truncate() const =0;
};


struct Exact : public Num
{
    bool IsExact() const;
};


struct Fixnum : public Exact
{
    long long Value;

    Fixnum(long long val);
    std::string BuildString() const;
    Datum* DeepCopy() const;
    long double Fraction() const;
    long long Truncate() const;
};


struct Inexact : public Num
{
    bool IsInexact() const;
}; 


struct Flonum : public Inexact
{
    long double Value;

    Flonum(long double val);
    std::string BuildString() const;
    Datum* DeepCopy() const;
    long double Fraction() const;
    long long Truncate() const;
};


struct String : public Atom
{
    std::string Value;

    String(const std::string& s);
    std::string BuildString() const;
    Datum* DeepCopy() const;
    bool IsString() const;
};


struct Symbol : public String
{
    Symbol(const std::string& s);
    std::string BuildString() const;
};


struct Timestamp : public Exact
{
    static Timestamp* Parse(const String& s);

    long Year;
    long Month;
    long Day;
    long Hour;
    long Minute;
    long Second;
    double Frac;

    Timestamp();
    Timestamp(long y, long m, long d, long h, long min, long s, double f);

    Timestamp& operator= (const Timestamp& other);

    std::string BuildString() const;
    Datum* DeepCopy() const;
    long double Fraction() const;
    bool IsTimestamp() const;
    long long Truncate() const;
};
END_HEUREKA;
