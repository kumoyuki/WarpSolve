#include "SExp.h"

#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>

BEGIN_HEUREKA;


///////////////////////////////////////////////////////////////////////////
// Datum::Reader::Error
//
Datum::Reader::Error::Error() : Line(1), Column(1) {}
Datum::Reader::Error::Error(const Error& e) : Line(e.Line), Column(e.Column), Message(e.Message) {}
Datum::Reader::Error::Error(size_t line, size_t col, const std::string msg)
  : Line(line), Column(col), Message(msg) {}


Datum::Reader::Error& Datum::Reader::Error::operator= (const Error& e) {
    Line = e.Line;
    Column = e.Column;
    Message = e.Message;
    return *this; }


///////////////////////////////////////////////////////////////////////////
// Datum::Reader
//
Datum::Reader::Reader(const std::string& text)
  : Cursor(text.begin()),
    End(text.end()),
    Line(1), Column(1)
{}


void Datum::Reader::AddError(std::string m) {
    Error e(Line, Column, m);
    Errors.push_back(e);
    return; }


void Datum::Reader::Deblank() {
    while(Cursor != End) {
        if(!isspace(*Cursor, Locale))
            break;

        if(*Cursor == '\n') 
            Line += 1;

        Cursor += 1; }

    return; }


bool Datum::Reader::Parse(void (Reader::* reader)()) {
    auto started = Cursor;
    size_t line = Line;
    size_t col = Column;
    auto errors = Errors;
    
    (this->*reader)();

    if(Errors.size() > 0 || Red == nullptr || Cursor == started) {
        Cursor = started;
        Line = line;
        Column = col;
        Errors = errors;
        return false; }

    return true; }


std::unique_ptr<Datum::Reader> Datum::Reader::Read(const std::string& text) {
    Reader* r = new Reader(text);
    r->ReadNext();
    return std::unique_ptr<Reader>(r); }
    

std::unique_ptr<Datum> Datum::Reader::ReadAtom() {
    if(*Cursor == '"')
        return std::unique_ptr<Atom>(new String(ReadQuoted('"')));

    else if(*Cursor == '|')
        return std::unique_ptr<Atom>(new Symbol(ReadQuoted('|')));

    else {
        std::string atom_text = ReadAtomicText();
        Datum::Reader details(atom_text);

        if(details.Parse(&Reader::ReadTimestamp))
            return std::unique_ptr<Datum>(details.Red.release());

        if(details.Parse(&Reader::ReadNumber))
            return std::unique_ptr<Datum>(details.Red.release());
        
        return std::unique_ptr<Datum>(new Symbol(atom_text)); }}


std::string Datum::Reader::ReadAtomicText() {
    auto start = Cursor;
    for(; Cursor != End 
          && !isspace(*Cursor, Locale)
          && *Cursor != '('
          && *Cursor != ')' 
          && *Cursor != '"' 
          && *Cursor != '|';
        Cursor++)
        if(*Cursor == '\\')
            Cursor += 1;

    return std::string(start, Cursor); }


void Datum::Reader::ReadList() {
    std::unique_ptr<List> l(new List);
    Cursor += 1;
    Deblank();
    while(*Cursor != ')') {
        ReadNext();
        if(Errors.size() > 0)
            break;
        l->Data.push_back(Red.release());
        Deblank(); }

    if(*Cursor == ')') {
        Cursor += 1;
        Red.reset(l.release()); }

    return; }


bool Datum::Reader::ReadNext() {
    Red.reset(nullptr);
    Deblank();
    if(Cursor == End || *Cursor == 0)
        return false;

    char c = *Cursor;
    if(c == '(')
        ReadList();
    else 
        Red.reset(ReadAtom().release());

    return Errors.size() == 0 && Red != nullptr; }


long long Datum::Reader::ReadNumberComponent() {
    signed sign = +1;
    if(*Cursor == '-')
        sign = -1;
    else if(*Cursor == '+')
        Cursor += 1;

    long long accum = 0;
    while(Cursor != End && isdigit(*Cursor, Locale)) {
        accum *= 10;
        accum += *Cursor - '0';
        Cursor += 1; }

    accum *= sign;
    return accum; }

#if defined(UNIX) // should be GCC probably
#define LOG10_TYPE long long
#define POW_TYPE long long
#else
#define LOG10_TYPE long double
#define POW_TYPE long double
#endif

void Datum::Reader::ReadNumber() {
    long long exponent = 0;
    long long fraction = 0;
    long long fixnum = ReadNumberComponent();
    
    if(Cursor == End) {
        Red.reset(new Fixnum(fixnum));
        return; }

    char next = *Cursor;
    if(next == '.') {
        fraction = ReadNumberComponent();
        if(fraction < 0) {
            AddError("negative fractional part");
            return; }
        else
            next = *Cursor; }
    
    if(next == 'e' || next == 'E')
        exponent = ReadNumberComponent();

    if(fraction == 0 && exponent == 0)
        Red.reset(new Fixnum(fixnum));
    else {
        long double inx = (long double) fixnum;
        if(fraction > 0) {
            long double digits = floor(log10((LOG10_TYPE)fraction) + 1);
            inx += (long double)fraction * pow(10, -digits); }

        if(exponent > 0)
            inx *= pow(10, (POW_TYPE)exponent);

        Red.reset(new Flonum(inx)); }

    return; }


std::string Datum::Reader::ReadQuoted(char c) {
    if(*Cursor != c)
        return "";

    Cursor += 1;
    auto start = Cursor;
    for(;Cursor != End && *Cursor != c; Cursor++)
        if(*Cursor == '\\')
            Cursor += 1;
    
    std::string atomic_text(start, Cursor);

    if(Cursor != End)
        Cursor += 1;

    return atomic_text; }


void Datum::Reader::ReadString() {
    Red.reset(new String(ReadQuoted('"')));
    return; }


void Datum::Reader::ReadSymbol() {
    std::string atomic_text;
    if(*Cursor == '|')
        atomic_text = ReadQuoted('|');
    else
        atomic_text = ReadAtomicText();

    Red.reset(new Symbol(atomic_text));
    return; }


void Datum::Reader::ReadTimestamp() {
    auto r = ReadTimestampComponent('-', "year");
    if(!r.first) return;
    long y = r.second;

    r = ReadTimestampComponent('-', "month");
    if(!r.first) return;
    long m = r.second;

    r = ReadTimestampComponent('T', "day");
    if(!r.first) return;
    long d = r.second;

    r = ReadTimestampComponent(':', "hour");
    if(!r.first) return;
    long h = r.second;

    r = ReadTimestampComponent(':', "minute");
    if(!r.first) return;
    long minute = r.second;

    ReadNumber();
    if(Red == nullptr)
        return;

    const Num* seconds = dynamic_cast<Num*>(Red.get());
    if(seconds == nullptr) {
        AddError("seconds were non-numeric");
        return; }

    long long s = seconds->Truncate();
    long double f = seconds->Fraction();

    Red.reset(new Timestamp(y, m, d, h, minute, (long)s, (double)f));
    return; }


std::pair<bool, long> Datum::Reader::ReadTimestampComponent(char delim, const char* unit) {
    long long c = ReadNumberComponent();
    if(Cursor == End || *Cursor != delim) {
        AddError(std::string("no timestamp component smaller than ") + unit);
        return std::pair<bool, long>(false, 0); }
    Cursor += 1;
    return std::pair<bool, long>(true, (long)c); }



///////////////////////////////////////////////////////////////////////////
// Datum
//
Datum::~Datum() {}


template <> std::unique_ptr<Datum> Datum::AsDatum(const double from) {
    return std::unique_ptr<Datum>(new Flonum(from)); }


template <> std::unique_ptr<Datum> Datum::AsDatum(const long from) {
    return std::unique_ptr<Datum>(new Fixnum(from)); }


template <> std::unique_ptr<Datum> Datum::AsDatum(const std::string from) {
    return std::unique_ptr<Datum>(new String(from)); }


template <> std::unique_ptr<Datum> Datum::AsDatum(FILE* f) {
    char buffer[512];
    std::string text;
    do {
        fgets(buffer, 512, f);
        text += buffer;
        } while(!feof(f));
    
    std::unique_ptr<Reader> r = Reader::Read(text);
    if(r == nullptr)
        return nullptr;
    else
        return std::move(r->Red); }


std::string Datum::AsString() const {
    if(Text.size() > 0)
        return Text;

    Text = BuildString();
    return Text; }


bool Datum::IsAtom() const { return false; }
bool Datum::IsList() const { return false; }
bool Datum::IsNumber() const { return false; }
bool Datum::IsString() const { return false; }
bool Datum::IsTimestamp() const { return false; }


///////////////////////////////////////////////////////////////////////////
// Atom
//
bool Atom::IsAtom() const { return true; }


///////////////////////////////////////////////////////////////////////////
// Exact
//
bool Exact::IsExact() const { return true; }


///////////////////////////////////////////////////////////////////////////
// Fixnum
//
Fixnum::Fixnum(long long val) : Value(val) {}


std::string Fixnum::BuildString() const {
    char buffer[32];
    sprintf(buffer, "%lld", Value);
    std::string text = buffer;
    return text; }


Datum* Fixnum::DeepCopy() const {
    return new Fixnum(Value); }


long double Fixnum::Fraction() const { return 0; }


long long Fixnum::Truncate() const { return Value; }


///////////////////////////////////////////////////////////////////////////
// Flonum
//
Flonum::Flonum(long double val) : Value(val) {}


std::string Flonum::BuildString() const {
    char buffer[32];
    sprintf(buffer, "%Lg", Value);
    std::string text = buffer;
    return text; }


Datum* Flonum::DeepCopy() const {
    return new Flonum(Value); }


long double Flonum::Fraction() const { return Value - Truncate(); }


long long Flonum::Truncate() const { return (long long)floor(Value); }


///////////////////////////////////////////////////////////////////////////
// Inexact
//
bool Inexact::IsInexact() const { return true; }


///////////////////////////////////////////////////////////////////////////
// List
//
List::~List() {
    std::for_each(std::begin(Data), std::end(Data), [] (Datum* d) { delete d; });
    Data.clear(); }


std::vector<Datum*>::const_iterator List::begin() const { return Data.begin(); }


std::string List::BuildString() const {
    std::string text;
    text += "(";
        
    std::for_each(Data.begin(), Data.end(),
                  [&,this] (const Datum* d) {
                      if(d == NULL)
                          text += "()";
                      else
                          text += d->AsString();

                      text += " "; });

    text = text.substr(0, Text.size() - 1);
    text += ")";
    return text; }


Datum* List::DeepCopy() const {
    List* copy = new List();
    std::for_each(std::begin(Data), std::end(Data),
                  [&] (const Datum* d) { copy->Data.push_back(d->DeepCopy()); });
    return copy; }


std::vector<Datum*>::const_iterator List::end() const { return Data.end(); }


bool List::IsList() const { return true; }


size_t List::Length() const { return Data.size(); }


Datum* List::Raw(size_t n) const {
    if(n >= Data.size())
        return NULL;

    auto i = Data.begin();
    while(n > 0) {
        i++;
        n -= 1; }

    return *i; }


const Datum& List::Ref(size_t n) const {
    Datum* d = Raw(n);
    return *d; }


void List::Set(size_t n, Datum& d) {
    if(n >= Data.size())
        return;

    if(Data[n] != nullptr)
        delete Data[0];

    Data[n] = &d; }


///////////////////////////////////////////////////////////////////////////
// Num
//
bool Num::IsExact() const { return false; }
bool Num::IsInexact() const { return false; }
bool Num::IsNumber() const { return true; }


///////////////////////////////////////////////////////////////////////////
// String
//
String::String(const std::string& s) : Value(s) {}


std::string String::BuildString() const {
    std::string text = "\"";
    text += Value; // FIXME: needs quotation...
    text += "\"";
    return text; }


Datum* String::DeepCopy() const {
    return new String(Value); }


bool String::IsString() const { return true; }


///////////////////////////////////////////////////////////////////////////
// Symbol
//
Symbol::Symbol(const std::string& s) : String(s) {}


std::string Symbol::BuildString() const {
    std::string text = Value; // FIXME: needs quotation...
    return text; }


///////////////////////////////////////////////////////////////////////////
// Timestamp
//
Timestamp::Timestamp()
  : Year(0), Month(0), Day(0),
    Hour(0), Minute(0), Second(0),
    Frac(0)
{}


Timestamp::Timestamp(long y, long m, long d, long h, long n, long s, double f)
  : Year(y), Month(m), Day(d),
    Hour(h), Minute(n), Second(s),
    Frac(f)
{}


std::string Timestamp::BuildString() const {
    char buffer[64];
    sprintf(buffer, "%04ld-%02ld-%02ldT%02ld:%02ld:%02ld",
            Year, Month, Day, Hour, Minute, Second);
    std::string text = buffer;
    return text; }


Datum* Timestamp::DeepCopy() const {
    return new Timestamp(Year, Month, Day, Hour, Minute, Second, Frac); }


long double Timestamp::Fraction() const { return Frac; }


bool Timestamp::IsTimestamp() const { return true; }


Timestamp& Timestamp::operator= (const Timestamp& other) {
    Year = other.Year;
    Month = other.Month;
    Day = other.Day;
    Hour = other.Hour;
    Minute = other.Minute;
    Second = other.Second;
    Frac = other.Frac;
    return *this; }


Timestamp* Timestamp::Parse(const String& s) {
    Datum::Reader reader(s.Value);
    if(!reader.ReadNext())
        return nullptr;

    if(reader.Red->IsTimestamp())
        return dynamic_cast<Timestamp*>(reader.Red.release());

    return nullptr; }


long long Timestamp::Truncate() const {
    static unsigned days_to[] = {
//    jan  feb  mar  apr  may  jun  jul  aug  sep  oct  nov  dec
        0,  31,  59,  89, 119, 150, 180, 211, 242, 272, 303, 334 };

    bool leap = Year % 4 == 0 && Year % 400 != 0;

    return Second
        + Minute * 60
        + Hour * 3600
        + Day * 86400
        + (days_to[Month] + (Month > 2 && leap ?1 :0))* 86400
        + (Year + Year / 4) * 365 * 86400
        ; }


END_HEUREKA;
