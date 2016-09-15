#ifndef _TYPE_H_
#define _TYPE_H_

#include <string>
#include <vector>

// some types
typedef int I4;
typedef short I2;
typedef char I1;
typedef unsigned int U4;
typedef unsigned short U2;
typedef unsigned char U1;
typedef double F8;
typedef float F4;


enum
{
  SENDFILE = 0,
  FIXED_STRUCT,
  MUTABLE_STRUCT
};

// some structs
typedef struct
{
  F8 f8;
  F4 f4;
  I4 i4;
  U4 u4;
  I2 i2;
  U2 u2;
  I1 i1;
  U1 u1;
}FIXED_LENGTH_STRUCT;

typedef struct
{
  I4 i4;
  U2 u2;
  std::string str;
  std::vector<std::string> vector_strList;
}MUTABLE_LENGTH_STRUCT;



#endif
