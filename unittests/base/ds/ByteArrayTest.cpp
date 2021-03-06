﻿// @copyright 2017-2018 zzu_softboy <zzu_softboy@163.com>
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Created by zzu_softboy on 2017/12/18.

#include "gtest/gtest.h"
#include <list>
#include <utility>
#include <tuple>
#include <vector>
#include <algorithm>

#include "pdk/base/ds/ByteArray.h"

using pdk::ds::ByteArrayData;
using pdk::ds::ByteArrayDataPtr;
using pdk::ds::ByteArray;

namespace
{

struct StaticByteArrays
{
   struct Standard 
   {
      ByteArrayData m_data;
      const char m_string[8];
   }m_standard;
   
   struct NotNullTerminated
   {
      ByteArrayData m_data;
      const char m_string[8];
   }m_notNullTerminated;
   
   struct Shifted {
      ByteArrayData m_data;
      const char m_dummy;  // added to change offset of string
      const char m_string[8];
   } m_shifted;
   
   struct ShiftedNotNullTerminated {
      ByteArrayData m_data;
      const char m_dummy;  // added to change offset of string
      const char m_string[8];
   } m_shiftedNotNullTerminated;
};

const StaticByteArrays statics = {
   {PDK_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER(4), "data"},
   {PDK_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER(4), "dataBAD"},
   {PDK_STATIC_BYTE_DATA_HEADER_INITIALIZER_WITH_OFFSET(4, sizeof(ByteArrayData) + sizeof(char)), 0, "data"},
   {PDK_STATIC_BYTE_DATA_HEADER_INITIALIZER_WITH_OFFSET(4, sizeof(ByteArrayData) + sizeof(char)), 0, "dataBAD"}
};

const ByteArrayDataPtr staticStandard = {
   const_cast<ByteArrayData *>(&statics.m_standard.m_data)
};

const ByteArrayDataPtr staticNotNullTerminated = {
   const_cast<ByteArrayData *>(&statics.m_notNullTerminated.m_data)
};

const ByteArrayDataPtr staticShifted = {
   const_cast<ByteArrayData *>(&statics.m_shifted.m_data)
};

const ByteArrayDataPtr staticShiftedNotNullTerminated = {
   const_cast<ByteArrayData *>(&statics.m_shiftedNotNullTerminated.m_data)
};

template <typename T>
const T &verify_zero_termination(const T &t)
{
   return t;
}


ByteArray verify_zero_termination(const ByteArray &array)
{
   ByteArray::DataPtr baDataPtr = const_cast<ByteArray &>(array).getDataPtr();
   if (baDataPtr->m_ref.isShared()
       || baDataPtr->m_offset != ByteArray().getDataPtr()->m_offset) {
      return array;
   }
   int baSize = array.size();
   char baTerminator = array.getConstRawData()[baSize];
   
   if ('\0' != baTerminator) {
      
   }
   
   // Skip mutating checks on shared strings
   if (baDataPtr->m_ref.isShared()) {
      return array;
   }
   
   const char *baData = array.getConstRawData();
   const ByteArray baCopy(baData, baSize); // Deep copy
   
   const_cast<char *>(baData)[baSize] = 'x';
   if ('x' != array.getConstRawData()[baSize]) {
      return "*** Failed to replace null-terminator in "
             "result ('" + array + "') ***";
   }
   if (array != baCopy) {
      return  "*** Result ('" + array + "') differs from its copy "
                                        "after null-terminator was replaced ***";
   }
   const_cast<char *>(baData)[baSize] = '\0'; // Restore sanity
   return array;
}

}

TEST(ByteArrayTest, testConstructor)
{
   
}

TEST(ByteArrayTest, testConstByteArray)
{
   const char *ptr = "abc";
   ByteArray carray = ByteArray::fromRawData(ptr, 3);
   ASSERT_EQ(carray.getConstRawData(), ptr);
   carray.squeeze();
   ASSERT_EQ(carray.getConstRawData(), ptr);
   carray.detach();
   ASSERT_EQ(carray.capacity(), 3);
   ASSERT_EQ(carray.size(), 3);
   ASSERT_NE(carray.getConstRawData(), ptr);
   ASSERT_EQ(carray.getConstRawData()[0], 'a');
   ASSERT_EQ(carray.getConstRawData()[1], 'b');
   ASSERT_EQ(carray.getConstRawData()[2], 'c');
   ASSERT_EQ(carray.getConstRawData()[3], '\0');
}

TEST(ByteArrayTest, testLeftJustified)
{
   ByteArray array;
   array = "PDK";
   ASSERT_EQ(array.leftJustified(5, '-'), ByteArray("PDK--"));
   ASSERT_EQ(array.leftJustified(4, '-'), ByteArray("PDK-"));
   ASSERT_EQ(array.leftJustified(4), ByteArray("PDK "));
   ASSERT_EQ(array.leftJustified(3), ByteArray("PDK"));
   ASSERT_EQ(array.leftJustified(2), ByteArray("PDK"));
   ASSERT_EQ(array.leftJustified(1), ByteArray("PDK"));
   ASSERT_EQ(array.leftJustified(0), ByteArray("PDK"));
   
   ByteArray n;
   ASSERT_TRUE(!n.leftJustified(3).isNull());
   ASSERT_EQ(array.leftJustified(4, ' ', true), ByteArray("PDK "));
   ASSERT_EQ(array.leftJustified(3, ' ', true), ByteArray("PDK"));
   ASSERT_EQ(array.leftJustified(2, ' ', true), ByteArray("PD"));
   ASSERT_EQ(array.leftJustified(1, ' ', true), ByteArray("P"));
   ASSERT_EQ(array.leftJustified(0, ' ', true), ByteArray(""));
   ASSERT_EQ(array, ByteArray("PDK"));
}

TEST(ByteArrayTest, testRightJustified)
{
   ByteArray array;
   array = "PDK";
   ASSERT_EQ(array.rightJustified(5, '-'), ByteArray("--PDK"));
   ASSERT_EQ(array.rightJustified(4, '-'), ByteArray("-PDK"));
   ASSERT_EQ(array.rightJustified(4), ByteArray(" PDK"));
   ASSERT_EQ(array.rightJustified(3), ByteArray("PDK"));
   ASSERT_EQ(array.rightJustified(2), ByteArray("PDK"));
   ASSERT_EQ(array.rightJustified(1), ByteArray("PDK"));
   ASSERT_EQ(array.rightJustified(0), ByteArray("PDK"));
   
   ByteArray n;
   ASSERT_TRUE(!n.rightJustified(3).isNull());
   ASSERT_EQ(array.rightJustified(4, '-', true), ByteArray("-PDK"));
   ASSERT_EQ(array.rightJustified(4, ' ', true), ByteArray(" PDK"));
   ASSERT_EQ(array.rightJustified(3, ' ', true), ByteArray("PDK"));
   ASSERT_EQ(array.rightJustified(2, ' ', true), ByteArray("PD"));
   ASSERT_EQ(array.rightJustified(1, ' ', true), ByteArray("P"));
   ASSERT_EQ(array.rightJustified(0, ' ', true), ByteArray(""));
   ASSERT_EQ(array, ByteArray("PDK"));
}

namespace
{

void prepare_prepend_data(std::list<ByteArray> &data)
{
   data.push_back(ByteArray(ByteArrayLiteral("data")));
   data.push_back(ByteArray(staticStandard));
   data.push_back(ByteArray(staticShifted));
   data.push_back(ByteArray(staticNotNullTerminated));
   data.push_back(ByteArray(staticShiftedNotNullTerminated));
   data.push_back(ByteArray("data"));
   data.push_back(ByteArray::fromRawData("data", 4));
   data.push_back(ByteArray::fromRawData("dataBAD", 4));
}

}

TEST(ByteArrayTest, testPrepend)
{
   ByteArray array("foo");
   ASSERT_EQ(array.prepend(static_cast<char *>(0)), ByteArray("foo"));
   ASSERT_EQ(array.prepend(ByteArray()), ByteArray("foo"));
   ASSERT_EQ(array.prepend("a"), ByteArray("afoo"));
   ASSERT_EQ(array.prepend("b"), ByteArray("bafoo"));
   ASSERT_EQ(array.prepend('c'), ByteArray("cbafoo"));
   ASSERT_EQ(array.prepend(-1, 'x'), ByteArray("cbafoo"));
   ASSERT_EQ(array.prepend(3, 'x'), ByteArray("xxxcbafoo"));
   ASSERT_EQ(array.prepend("\0 ", 2), ByteArray::fromRawData("\0 xxxcbafoo", 11));
}

TEST(ByteArrayTest, testPrependExtend)
{
   std::list<ByteArray> data;
   prepare_prepend_data(data);
   std::list<ByteArray>::iterator begin = data.begin();
   std::list<ByteArray>::iterator end = data.end();
   while (begin != end) {
      ByteArray array = *begin;
      ASSERT_EQ(ByteArray().prepend(array), ByteArray("data"));
      ASSERT_EQ(ByteArray("").prepend(array), ByteArray("data"));
      
      ASSERT_EQ(array.prepend(static_cast<char *>(0)), ByteArray("data"));
      ASSERT_EQ(array.prepend(ByteArray()), ByteArray("data"));
      ASSERT_EQ(array.prepend("a"), ByteArray("adata"));
      ASSERT_EQ(array.prepend(ByteArray("b")), ByteArray("badata"));
      ASSERT_EQ(array.prepend('c'), ByteArray("cbadata"));
      ASSERT_EQ(array.prepend(-1, 'x'), ByteArray("cbadata"));
      ASSERT_EQ(array.prepend(3, 'x'), ByteArray("xxxcbadata"));
      ASSERT_EQ(array.prepend("\0 ", 2), ByteArray::fromRawData("\0 xxxcbadata", 12));
      ASSERT_EQ(array.size(), 12);
      ++begin;
   }
   
}

TEST(ByteArrayTest, testAppend)
{
   ByteArray array("foo");
   ASSERT_EQ(array.append(static_cast<char *>(0)), ByteArray("foo"));
   ASSERT_EQ(array.append(ByteArray()), ByteArray("foo"));
   ASSERT_EQ(array.append("a"), ByteArray("fooa"));
   ASSERT_EQ(array.append("b"), ByteArray("fooab"));
   ASSERT_EQ(array.append('c'), ByteArray("fooabc"));
   ASSERT_EQ(array.append(-1, 'x'), ByteArray("fooabc"));
   ASSERT_EQ(array.append(3, 'x'), ByteArray("fooabcxxx"));
   ASSERT_EQ(array.append("\0"), ByteArray("fooabcxxx"));
   ASSERT_EQ(array.append("\0", 1), ByteArray::fromRawData("fooabcxxx", 10));
   ASSERT_EQ(array.size(), 10);
}

TEST(ByteArrayTest, testAppendExtended)
{
   std::list<ByteArray> data;
   prepare_prepend_data(data);
   std::list<ByteArray>::iterator begin = data.begin();
   std::list<ByteArray>::iterator end = data.end();
   while (begin != end) {
      ByteArray array = *begin;
      ASSERT_EQ(ByteArray().append(array), ByteArray("data"));
      ASSERT_EQ(ByteArray("").append(array), ByteArray("data"));
      
      ASSERT_EQ(array.append(static_cast<char *>(0)), ByteArray("data"));
      ASSERT_EQ(array.append(ByteArray()), ByteArray("data"));
      ASSERT_EQ(array.append("a"), ByteArray("dataa"));
      ASSERT_EQ(array.append(ByteArray("b")), ByteArray("dataab"));
      ASSERT_EQ(array.append('c'), ByteArray("dataabc"));
      ASSERT_EQ(array.append(-1, 'x'), ByteArray("dataabc"));
      ASSERT_EQ(array.append(3, 'x'), ByteArray("dataabcxxx"));
      ASSERT_EQ(array.append("\0"), ByteArray("dataabcxxx"));
      ASSERT_EQ(array.append("\0", 1), ByteArray::fromRawData("dataabcxxx\0 ", 11));
      ASSERT_EQ(array.size(), 11);
      ++begin;
   }
}

TEST(ByteArrayTest, testMid)
{
   ByteArray data("zzu_softboy");
   ASSERT_EQ(data.mid(0, 3), ByteArray("zzu"));
   ASSERT_EQ(data.mid(0, -1), ByteArray("zzu_softboy"));
   ASSERT_EQ(data.mid(1, -1), ByteArray("zu_softboy"));
   ASSERT_EQ(data.mid(-10, 1), ByteArray());
   ASSERT_EQ(data.mid(-10, -1), ByteArray("zzu_softboy"));
   ASSERT_EQ(data.mid(-11, 22), ByteArray("zzu_softboy"));
   ASSERT_EQ(data.mid(-11, 23), ByteArray("zzu_softboy"));
   ASSERT_EQ(data.mid(-5, 6), ByteArray("z"));
   ASSERT_EQ(data.mid(0, 11), ByteArray("zzu_softboy"));
}

TEST(ByteArrayTest, testStartsWith)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray, bool>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), true));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(""), true));
   data.push_back(std::make_tuple(ByteArray(), ByteArray("hello"), false));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray(), true));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray(""), true));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray("h"), false));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray("h"), true));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray("hello"), true));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray(""), true));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray("hellohello"), false));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray(""), true));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray left = std::get<0>(item);
      ByteArray right = std::get<1>(item);
      bool result = std::get<2>(item);
      ASSERT_TRUE(left.startsWith(right) == result);
      if (right.isNull()) {
         ASSERT_TRUE(left.startsWith(static_cast<char *>(0)) == result);
      } else {
         ASSERT_TRUE(left.startsWith(right.getRawData()) == result);
      }
      ++begin;
   }
}

TEST(ByteArrayTest, testStartsWithChar)
{
   ASSERT_TRUE(ByteArray("hello").startsWith('h'));
   ASSERT_FALSE(ByteArray("hello").startsWith('\0'));
   ASSERT_FALSE(ByteArray("hello").startsWith('o'));
   ASSERT_TRUE(ByteArray("h").startsWith('h'));
   ASSERT_FALSE(ByteArray("h").startsWith('\0'));
   ASSERT_FALSE(ByteArray("h").startsWith('o'));
   ASSERT_FALSE(ByteArray("hello").startsWith('l'));
   ASSERT_FALSE(ByteArray("").startsWith('h'));
   ASSERT_FALSE(ByteArray("").startsWith('\0'));
   ASSERT_FALSE(ByteArray().startsWith('o'));
   ASSERT_FALSE(ByteArray().startsWith('\0'));
}

TEST(ByteArrayTest, testEndsWith)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray, bool>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), true));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(""), true));
   data.push_back(std::make_tuple(ByteArray(), ByteArray("hello"), false));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray(), true));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray(""), true));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray("h"), false));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray("o"), true));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray("hello"), true));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray(""), true));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray("hellohello"), false));
   data.push_back(std::make_tuple(ByteArray("hello"), ByteArray(""), true));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray left = std::get<0>(item);
      ByteArray right = std::get<1>(item);
      bool result = std::get<2>(item);
      ASSERT_TRUE(left.endsWith(right) == result);
      if (right.isNull()) {
         ASSERT_TRUE(left.endsWith(static_cast<char *>(0)) == result);
      } else {
         ASSERT_TRUE(left.endsWith(right.getRawData()) == result);
      }
      ++begin;
   }
}

TEST(ByteArrayTest, testEndsWithChar)
{
   ASSERT_TRUE(ByteArray("hello").endsWith('o'));
   ASSERT_FALSE(ByteArray("hello").endsWith('\0'));
   ASSERT_FALSE(ByteArray("hello").endsWith('h'));
   ASSERT_TRUE(ByteArray("h").endsWith('h'));
   ASSERT_FALSE(ByteArray("h").endsWith('\0'));
   ASSERT_FALSE(ByteArray("h").endsWith('o'));
   ASSERT_FALSE(ByteArray("hello").endsWith('l'));
   ASSERT_FALSE(ByteArray("").endsWith('h'));
   ASSERT_FALSE(ByteArray("").endsWith('\0'));
   ASSERT_FALSE(ByteArray("").endsWith('a'));
   ASSERT_FALSE(ByteArray().endsWith('a'));
   ASSERT_FALSE(ByteArray().endsWith('\0'));
}

TEST(ByteArrayTest, testIterators)
{
   {
      ByteArray array;
      ByteArray::iterator begin = array.begin();
      ByteArray::iterator end = array.end();
      ASSERT_EQ(begin, end);
   }
   {
      ByteArray array("a");
      ByteArray::iterator begin = array.begin();
      ByteArray::iterator end = array.end();
      ASSERT_NE(begin, end);
      ++begin;
      ASSERT_EQ(begin, end);
   }
   {
      std::list<char> expected{'a', 'b', 'c'};
      std::list<char> actual;
      ByteArray array("abc");
      ByteArray::iterator begin = array.begin();
      ByteArray::iterator end = array.end();
      while (begin != end) {
         actual.push_back(*begin);
         ++begin;
      }
      ASSERT_EQ(actual, expected);
   }
   {
      std::list<char> expected{'a', 'b', 'c'};
      std::list<char> actual;
      const ByteArray array("abc");
      ByteArray::const_iterator begin = array.begin();
      ByteArray::const_iterator end = array.end();
      while (begin != end) {
         actual.push_back(*begin);
         ++begin;
      }
      ASSERT_EQ(actual, expected);
   }
   {
      std::list<char> expected{'a', 'b', 'c'};
      std::list<char> actual;
      const ByteArray array("abc");
      ByteArray::const_iterator begin = array.cbegin();
      ByteArray::const_iterator end = array.cend();
      while (begin != end) {
         actual.push_back(*begin);
         ++begin;
      }
      ASSERT_EQ(actual, expected);
   }
}

TEST(ByteArrayTest, testReverseIterators)
{
   ByteArray data = "abcd";
   ByteArray reverseData = data;
   std::reverse(reverseData.begin(), reverseData.end());
   const ByteArray &constReverseData = reverseData;
   ASSERT_TRUE(std::equal(data.begin(), data.end(), reverseData.rbegin()));
   ASSERT_TRUE(std::equal(data.begin(), data.end(), reverseData.crbegin()));
   ASSERT_TRUE(std::equal(data.begin(), data.end(), constReverseData.rbegin()));
   ASSERT_TRUE(std::equal(reverseData.rbegin(), reverseData.rend(), data.begin()));
   ASSERT_TRUE(std::equal(reverseData.crbegin(), reverseData.crend(), data.begin()));
   ASSERT_TRUE(std::equal(constReverseData.rbegin(), constReverseData.rend(), data.begin()));
}

TEST(ByteArrayTest, testInsert)
{
   ByteArray array("Meal");
   ASSERT_EQ(array.insert(1, ByteArray("ontr")), ByteArray("Montreal"));
   ASSERT_EQ(array.insert(array.size(), ByteArray("foo")), ByteArray("Montrealfoo"));
   
   array = "13";
   ASSERT_EQ(array.insert(1, ByteArray("2")), ByteArray("123"));
   
   array = "ac";
   ASSERT_EQ(array.insert(1, 'b'), ByteArray("abc"));
   
   array = "ac";
   ASSERT_EQ(array.insert(-1, 3, 'b'), ByteArray("ac"));
   ASSERT_EQ(array.insert(1, 3, 'x'), ByteArray("axxxc"));
   ASSERT_EQ(array.insert(6, 3, 'x'), ByteArray("axxxc xxx"));
   ASSERT_EQ(array.size(), 9);
   
   array = "ikl";
   ASSERT_EQ(array.insert(1, 'j'), ByteArray("ijkl"));
   ASSERT_EQ(array.size(), 4);
   
   array = "ab";
   ASSERT_EQ(array.insert(1, "\0X\0", 3), ByteArray::fromRawData("a\0X\0b", 5));
   ASSERT_EQ(array.size(), 5);
}

TEST(ByteArrayTest, testInsertExtended)
{
   std::list<ByteArray> data;
   prepare_prepend_data(data);
   std::list<ByteArray>::iterator begin = data.begin();
   std::list<ByteArray>::iterator end = data.end();
   while (begin != end) {
      ByteArray array = *begin;
      ASSERT_EQ(array.insert(1, "i"), ByteArray("diata"));
      ASSERT_EQ(array.insert(1, 3, 'x'), ByteArray("dxxxiata"));
      ASSERT_EQ(array.size(), 8);
      ++begin;
   }
   
}

TEST(ByteArrayTest, testToUpperAndLowercase)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray, ByteArray>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), ByteArray()));
   data.push_back(std::make_tuple(ByteArrayLiteral("Hello World"), 
                                  ByteArrayLiteral("HELLO WORLD"), 
                                  ByteArrayLiteral("hello world")));
   data.push_back(std::make_tuple(ByteArrayLiteral("Hello World, this is a STRING"),
                                  ByteArrayLiteral("HELLO WORLD, THIS IS A STRING"), 
                                  ByteArrayLiteral("hello world, this is a string")));
   data.push_back(std::make_tuple(ByteArrayLiteral("R\311sum\351"),
                                  ByteArrayLiteral("R\311SUM\311"), 
                                  ByteArrayLiteral("r\351sum\351")));
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray input = std::get<0>(item);
      ByteArray uppercase = std::get<1>(item);
      ByteArray lowercase = std::get<2>(item);
      ASSERT_EQ(lowercase.toLower(), lowercase);
      ASSERT_EQ(uppercase.toUpper(), uppercase);
      ASSERT_EQ(input.toUpper(), uppercase);
      ASSERT_EQ(input.toLower(), lowercase);
      
      ByteArray copy = input;
      ASSERT_TRUE(copy.isSharedWith(input));
      ASSERT_EQ(std::move(copy).toUpper(), uppercase);
      copy = input;
      copy.detach();
      ASSERT_FALSE(copy.isSharedWith(input));
      ASSERT_EQ(std::move(copy).toUpper(), uppercase);
      
      copy = input;
      ASSERT_TRUE(copy.isSharedWith(input));
      ASSERT_EQ(std::move(copy).toLower(), lowercase);
      copy = input;
      copy.detach();
      ASSERT_FALSE(copy.isSharedWith(input));
      ASSERT_EQ(std::move(copy).toLower(), lowercase);
      
      copy = lowercase;
      ASSERT_TRUE(copy.isSharedWith(lowercase));
      ASSERT_EQ(std::move(copy).toLower(), lowercase);
      copy = input;
      copy.detach();
      ASSERT_FALSE(copy.isSharedWith(lowercase));
      ASSERT_EQ(std::move(copy).toLower(), lowercase);
      
      copy = uppercase;
      ASSERT_TRUE(copy.isSharedWith(uppercase));
      ASSERT_EQ(std::move(copy).toUpper(), uppercase);
      copy = input;
      copy.detach();
      ASSERT_FALSE(copy.isSharedWith(uppercase));
      ASSERT_EQ(std::move(copy).toUpper(), uppercase);
      ++begin;
   }
}

TEST(ByteArrayTest, testIndexOf)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray, int, int>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("a"), 0, 0));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("A"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("a"), 1, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("A"), 1, -1));
   
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("b"), 0, 1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("B"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("b"), 1, 1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("B"), 1, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("b"), 2, -1));
   
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("c"), 0, 2));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("C"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("c"), 1, 2));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("C"), 1, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("c"), 2, 2));
   
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("bc"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("Bc"), 0, 1));
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("bC"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("BC"), 0, -1));
   
   static const char h19[] = {'x', 0x00, (char)0xe7, 0x25, 0x1c, 0x0a};
   static const char n19[] = {0x00, 0x00, 0x01, 0x00};
   
   data.push_back(std::make_tuple(ByteArray(h19, sizeof(h19)), ByteArray(n19, sizeof(n19)), 0, -1));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray("x"), 0, -1));
   data.push_back(std::make_tuple(ByteArray(), ByteArray("x"), 0, -1));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), 0, 0));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(""), 0, 0));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray(), 0, 0));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray(""), 0, 0));
   
   ByteArray veryBigHaystack(500, 'a');
   veryBigHaystack += 'B';
   data.push_back(std::make_tuple(veryBigHaystack, veryBigHaystack, 0, 0));
   data.push_back(std::make_tuple(ByteArray(veryBigHaystack + 'c'), ByteArray(veryBigHaystack), 0, 0));
   data.push_back(std::make_tuple(ByteArray('c' + veryBigHaystack), ByteArray(veryBigHaystack), 0, 1));
   data.push_back(std::make_tuple(ByteArray(veryBigHaystack), ByteArray(veryBigHaystack + 'c'), 0, -1));
   data.push_back(std::make_tuple(ByteArray(veryBigHaystack), ByteArray('c' + veryBigHaystack), 0, -1));
   data.push_back(std::make_tuple(ByteArray('d' + veryBigHaystack), ByteArray('c' + veryBigHaystack), 0, -1));
   data.push_back(std::make_tuple(ByteArray(veryBigHaystack + 'c'), ByteArray('c' + veryBigHaystack), 0, -1));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray haystack = std::get<0>(item);
      ByteArray needle = std::get<1>(item);
      int startpos = std::get<2>(item);
      int expected = std::get<3>(item);
      
      bool hasNull = needle.contains('\0');
      ASSERT_EQ(haystack.indexOf(needle, startpos), expected);
      if (!hasNull) {
         ASSERT_EQ(haystack.indexOf(needle.getRawData(), startpos), expected);
      }
      if (needle.size() == 1) {
         ASSERT_EQ(haystack.indexOf(needle.at(0), startpos), expected);
      }
      if (startpos == 0) {
         ASSERT_EQ( haystack.indexOf(needle), expected );
         if (!hasNull) {
            ASSERT_EQ(haystack.indexOf(needle.getRawData()), expected);
         }
         
         if (needle.size() == 1) {
            ASSERT_EQ(haystack.indexOf(needle.at(0)), expected);
         }
      }
      ++begin;
   }
}

TEST(ByteArrayTest, testLastIndexOf)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray, int, int>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("a"), 0, 0));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("A"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("a"), 1, 0));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("A"), 1, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("a"), -1, 0));
   
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("b"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("B"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("b"), 1, 1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("B"), 1, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("b"), 2, 1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("b"), -1, 1));
   
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("c"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("C"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("c"), 1, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("C"), 1, -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("c"), 2, 2));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("c"), -1, 2));
   
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("bc"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("Bc"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("Bc"), 2, 1));
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("Bc"), 1, 1));
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("Bc"), -1, 1));
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("bC"), 0, -1));
   data.push_back(std::make_tuple(ByteArray("aBc"), ByteArray("BC"), 0, -1));
   
   static const char h25[] = {0x00, (char)0xbc, 0x03, 0x10, 0x0a };
   static const char n25[] = {0x00, 0x00, 0x01, 0x00};
   data.push_back(std::make_tuple(ByteArray(h25, sizeof(h25)), ByteArray(n25, sizeof(n25)), 0, -1));
   
   data.push_back(std::make_tuple(ByteArray(""), ByteArray("x"), -1, -1));
   data.push_back(std::make_tuple(ByteArray(), ByteArray("x"), -1, -1));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), -1, 0));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(""), -1, 0));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray(), -1, 0));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray(""), -1, 0));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray haystack = std::get<0>(item);
      ByteArray needle = std::get<1>(item);
      int startpos = std::get<2>(item);
      int expected = std::get<3>(item);
      
      bool hasNull = needle.contains('\0');
      ASSERT_EQ(haystack.lastIndexOf(needle, startpos), expected);
      if (!hasNull) {
         ASSERT_EQ(haystack.lastIndexOf(needle.getRawData(), startpos), expected);
      }
      
      if (needle.size() == 1) {
         ASSERT_EQ(haystack.lastIndexOf(needle.at(0), startpos), expected);
      }
      if (startpos == -1) {
         ASSERT_EQ(haystack.lastIndexOf(needle), expected );
         if (!hasNull) {
            ASSERT_EQ(haystack.lastIndexOf(needle.getRawData()), expected);
         }
         if (needle.size() == 1) {
            ASSERT_EQ(haystack.lastIndexOf(needle.at(0)), expected);
         }
         
      }
      
      ++begin;
   }
}

TEST(ByteArrayTest, testReplace)
{
   using DataType = std::list<std::tuple<ByteArray, int, int, ByteArray, ByteArray>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray("Say yes!"), 4, 3, 
                                  ByteArray("no"), ByteArray("Say no!")));
   data.push_back(std::make_tuple(ByteArray("rock and roll"), 5, 3, 
                                  ByteArray("&"), ByteArray("rock & roll")));
   data.push_back(std::make_tuple(ByteArray("foo"), 3, 0, 
                                  ByteArray("bar"), ByteArray("foobar")));
   data.push_back(std::make_tuple(ByteArray(), 0, 0, 
                                  ByteArray(), ByteArray()));
   data.push_back(std::make_tuple(ByteArray(), 3, 0, 
                                  ByteArray("hi"), ByteArray("   hi")));
   
   data.push_back(std::make_tuple(ByteArray("abcdef"), 3, 12, 
                                  ByteArray("abcdefghijkl"), ByteArray("abcabcdefghijkl")));
   data.push_back(std::make_tuple(ByteArray("abcdef"), 3, 4, 
                                  ByteArray("abcdefghijkl"), ByteArray("abcabcdefghijkl")));
   data.push_back(std::make_tuple(ByteArray("abcdef"), 3, 4, 
                                  ByteArray("abcdefghijkl"), ByteArray("abcabcdefghijkl")));
   data.push_back(std::make_tuple(ByteArray("abcdef"), 3, 2, 
                                  ByteArray("abcdefghijkl"), ByteArray("abcabcdefghijklf")));
   data.push_back(std::make_tuple(ByteArray("abcdef"), 2, 2, 
                                  ByteArray("xx"), ByteArray("abxxef")));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray src = std::get<0>(item);
      int pos = std::get<1>(item);
      int length = std::get<2>(item);
      ByteArray after = std::get<3>(item);
      ByteArray expected = std::get<4>(item);
      
      ByteArray str1 = src;
      ByteArray str2 = src;
      ASSERT_STREQ(str1.replace(pos, length, after).getConstRawData(), expected.getConstRawData());
      ASSERT_STREQ(str2.replace(pos, length, after.getRawData()), expected);
      ++begin;
   }
}

TEST(ByteArrayTest, testReplaceWithSpecifiedLength)
{
   const char after[] = "zxc\0vbnmqwert";
   int lenAfter = 6;
   ByteArray ba("abcdefghjk");
   ba.replace(0, 2, after, lenAfter);
   
   const char rawexpected[] = "zxc\0vbcdefghjk";
   ByteArray expected(rawexpected, sizeof(rawexpected) - 1);
   ASSERT_EQ(ba, expected);
}

TEST(ByteArrayTest, testToBase64)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray(""), ByteArray("")));
   data.push_back(std::make_tuple(ByteArray("1"), ByteArray("MQ==")));
   data.push_back(std::make_tuple(ByteArray("12"), ByteArray("MTI=")));
   data.push_back(std::make_tuple(ByteArray("123"), ByteArray("MTIz")));
   data.push_back(std::make_tuple(ByteArray("1234"), ByteArray("MTIzNA==")));
   
   data.push_back(std::make_tuple(ByteArray("\n"), ByteArray("Cg==")));
   data.push_back(std::make_tuple(ByteArray("a\n"), ByteArray("YQo=")));
   data.push_back(std::make_tuple(ByteArray("ab\n"), ByteArray("YWIK")));
   data.push_back(std::make_tuple(ByteArray("abc\n"), ByteArray("YWJjCg==")));
   data.push_back(std::make_tuple(ByteArray("abcd\n"), ByteArray("YWJjZAo=")));
   data.push_back(std::make_tuple(ByteArray("abcde\n"), ByteArray("YWJjZGUK")));
   data.push_back(std::make_tuple(ByteArray("abcdef\n"), ByteArray("YWJjZGVmCg==")));
   data.push_back(std::make_tuple(ByteArray("abcdefg\n"), ByteArray("YWJjZGVmZwo=")));
   data.push_back(std::make_tuple(ByteArray("abcdefgh\n"), ByteArray("YWJjZGVmZ2gK")));
   
   ByteArray ba;
   ba.resize(256);
   for (int i = 0; i < 256; ++i) {
      ba[i] = i;
   }
   
   data.push_back(std::make_tuple(ba, ByteArray("AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==")));
   data.push_back(std::make_tuple(ByteArray("foo\0bar", 7), ByteArray("Zm9vAGJhcg==")));
   data.push_back(std::make_tuple(ByteArray("f\xd1oo\x9ctar"), ByteArray("ZtFvb5x0YXI=")));
   data.push_back(std::make_tuple(ByteArray("\"\0\0\0\0\0\0\"", 8), ByteArray("IgAAAAAAACI="))); 
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray rawData = std::get<0>(item);
      ByteArray base64 = std::get<1>(item);
      ByteArray decoded = ByteArray::fromBase64(base64);
      ASSERT_EQ(decoded, rawData);
      
      ByteArray encoded = rawData.toBase64();
      ASSERT_EQ(encoded, base64);
      encoded = rawData.toBase64(ByteArray::Base64Encoding);
      ASSERT_EQ(encoded, base64);
      
      ByteArray base64noequals = base64;
      base64noequals.replace('=', "");
      encoded = rawData.toBase64(ByteArray::Base64Encoding | ByteArray::OmitTrailingEquals);
      ASSERT_EQ(encoded, base64noequals);
      
      ByteArray base64url = base64;
      base64url.replace('/', '_').replace('+', '-');
      encoded = rawData.toBase64(ByteArray::Base64UrlEncoding);
      ASSERT_EQ(encoded, base64url);
      
      ByteArray base64urlnoequals = base64url;
      base64urlnoequals.replace('=', "");
      encoded = rawData.toBase64(ByteArray::Base64UrlEncoding | ByteArray::OmitTrailingEquals);
      ASSERT_EQ(encoded, base64urlnoequals);
      
      ++begin;
   }
}

TEST(ByteArrayTest, testFromBase64)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray>>;
   DataType data;
   
   data.push_back(std::make_tuple(ByteArray(""), ByteArray("  ")));
   data.push_back(std::make_tuple(ByteArray("1"), ByteArray("MQ")));
   data.push_back(std::make_tuple(ByteArray("12"), ByteArray("MTI       ")));
   data.push_back(std::make_tuple(ByteArray("123"), ByteArray("M=TIz")));
   data.push_back(std::make_tuple(ByteArray("1234"), ByteArray("MTI zN A ")));
   
   data.push_back(std::make_tuple(ByteArray("\n"), ByteArray("Cg")));
   data.push_back(std::make_tuple(ByteArray("a\n"), ByteArray("======YQo=")));
   data.push_back(std::make_tuple(ByteArray("ab\n"), ByteArray("Y\nWIK")));
   data.push_back(std::make_tuple(ByteArray("abc\n"), ByteArray("YWJjCg==")));
   data.push_back(std::make_tuple(ByteArray("abcd\n"), ByteArray("YWJ\1j\x9cZAo=")));
   data.push_back(std::make_tuple(ByteArray("abcde\n"), ByteArray("YW JjZ\n G\tUK")));
   data.push_back(std::make_tuple(ByteArray("abcdef\n"), ByteArray("YWJjZGVmCg=")));
   data.push_back(std::make_tuple(ByteArray("abcdefg\n"), ByteArray("YWJ\rjZGVmZwo")));
   data.push_back(std::make_tuple(ByteArray("abcdefgh\n"), ByteArray("YWJjZGVmZ2gK")));
   
   ByteArray ba;
   ba.resize(256);
   for (int i = 0; i < 256; ++i) {
      ba[i] = i;
   }
   data.push_back(std::make_tuple(ba, ByteArray("AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Nj\n"
                                                "c4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1u\n"
                                                "b3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpa\n"
                                                "anqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd\n"
                                                "3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==                            ")));
   
   data.push_back(std::make_tuple(ByteArray("foo\0bar", 7), ByteArray("Zm9vAGJhcg")));
   data.push_back(std::make_tuple(ByteArray("f\xd1oo\x9ctar"), ByteArray("ZtFvb5x0YXI=")));
   data.push_back(std::make_tuple(ByteArray("\"\0\0\0\0\0\0\"", 8), ByteArray("IgAAAAAAACI")));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray rawData = std::get<0>(item);
      ByteArray base64 = std::get<1>(item);
      ByteArray decoded = ByteArray::fromBase64(base64);
      ASSERT_EQ(rawData, decoded);
      decoded = ByteArray::fromBase64(base64, ByteArray::Base64Encoding);
      ASSERT_EQ(rawData, decoded);
      // try "base64url" encoding
      ByteArray base64url = base64;
      base64url.replace('/', '_').replace('+', '-');
      decoded = ByteArray::fromBase64(base64url, ByteArray::Base64UrlEncoding);
      ASSERT_EQ(decoded, rawData);
      if (base64 != base64url) {
         // check that the invalid decodings fail
         decoded = ByteArray::fromBase64(base64, ByteArray::Base64UrlEncoding);
         ASSERT_FALSE(decoded == rawData);
         decoded = ByteArray::fromBase64(base64url, ByteArray::Base64Encoding);
         ASSERT_FALSE(decoded == rawData);
      }
      ++begin;
   }
}

TEST(ByteArrayTest, testChop)
{
   using DataType = std::list<std::tuple<ByteArray, int, ByteArray>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray("short1"), 128, ByteArray()));
   data.push_back(std::make_tuple(ByteArray("short2"), static_cast<int>(std::strlen("short2")), ByteArray()));
   data.push_back(std::make_tuple(ByteArray("abcdef\0foo", 10), 2, ByteArray("abcdef\0f", 8)));
   data.push_back(std::make_tuple(ByteArray("STARTTLS\r\n"), 2, ByteArray("STARTTLS")));
   data.push_back(std::make_tuple(ByteArray(""), 1, ByteArray()));
   data.push_back(std::make_tuple(ByteArray("foo"), 0, ByteArray("foo")));
   data.push_back(std::make_tuple(ByteArray(0), 28, ByteArray()));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray src = std::get<0>(item);
      int choplength = std::get<1>(item);
      ByteArray expected = std::get<2>(item);
      src.chop(choplength);
      ASSERT_EQ(src, expected);
      ++begin;
   }
}

TEST(ByteArrayTest, testSplit)
{
   using DataType = std::list<std::tuple<ByteArray, int>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray("-rw-r--r--  1 0  0  519240 Jul  9  2002 bigfile"), 14u));
   data.push_back(std::make_tuple(ByteArray("abcde"), 1u));
   data.push_back(std::make_tuple(ByteArray(""), 1u));
   data.push_back(std::make_tuple(ByteArray(" "), 2u));
   data.push_back(std::make_tuple(ByteArray("  "), 3u));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray sample = std::get<0>(item);
      uint size = std::get<1>(item);
      std::list<ByteArray> list = sample.split(' ');
      ASSERT_EQ(list.size(), static_cast<size_t>(size));
      ++begin;
   }
}

TEST(ByteArrayTest, testSwap)
{
   ByteArray b1 = "b1";
   ByteArray b2 = "b2";
   b1.swap(b2);
   ASSERT_EQ(b1, ByteArray("b2"));
   ASSERT_EQ(b2, ByteArray("b1"));
}

TEST(ByteArrayTest, testRepeatedSignature)
{
   const ByteArray string;
   PDK_UNUSED(string.repeated(9));
}

TEST(ByteArrayTest, testRepeated)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray, int>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), 0));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), -1004));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), 1));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), 5));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray(), -1004));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray(), -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray(), 0));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("abc"), 1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("abcabc"), 2));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("abcabcabc"), 3));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("abcabcabcabc"), 4));
   data.push_back(std::make_tuple(ByteArray(staticNotNullTerminated), ByteArray("datadatadatadata"), 4));
   data.push_back(std::make_tuple(ByteArray(staticStandard), ByteArray("datadatadatadata"), 4));
   data.push_back(std::make_tuple(ByteArray(staticShiftedNotNullTerminated), ByteArray("datadatadatadata"), 4));
   data.push_back(std::make_tuple(ByteArray(staticShifted), ByteArray("datadatadatadata"), 4));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray string = std::get<0>(item);
      ByteArray expected = std::get<1>(item);
      int count = std::get<2>(item);
      
      ASSERT_EQ(string.repeated(count), expected);
      ++begin;
   }
}

TEST(ByteArrayTest, testByteRefDetacting)
{
   {
      ByteArray str = "abc";
      ByteArray copy = str;
      copy[0] = 'A';
      ASSERT_EQ(str, ByteArray("abc"));
      ASSERT_EQ(copy, ByteArray("Abc"));
   }
   {
      char buf[] = {'a', 'b', 'c'};
      ByteArray str = ByteArray::fromRawData(buf, 3);
      str[0] = 'A';
      ASSERT_EQ(buf[0], static_cast<char>('a'));
   }
   {
      static const char buf[] = {'a', 'b', 'c'};
      ByteArray str = ByteArray::fromRawData(buf, 3);
      str[0] = 'A';
      ASSERT_EQ(buf[0], static_cast<char>('a'));
   }
}

TEST(ByteArrayTest, testReserve)
{
   int capacity = 100;
   ByteArray qba;
   qba.reserve(capacity);
   ASSERT_TRUE(qba.capacity() == capacity);
   char *data = qba.getRawData();
   for (int i = 0; i < capacity; i++) {
      qba.resize(i);
      ASSERT_TRUE(capacity == qba.capacity());
      ASSERT_TRUE(data == qba.getRawData());
   }
   qba.resize(capacity);
   ByteArray copy = qba;
   qba.reserve(capacity / 2);
   ASSERT_EQ(qba.size(), capacity); // we didn't shrink the size!
   ASSERT_EQ(qba.capacity(), capacity);
   ASSERT_EQ(copy.capacity(), capacity);
   
   qba = copy;
   qba.reserve(capacity * 2);
   ASSERT_EQ(qba.size(), capacity);
   ASSERT_EQ(qba.capacity(), capacity * 2);
   ASSERT_EQ(copy.capacity(), capacity);
   ASSERT_TRUE(qba.getConstRawData() != data);
   
   ByteArray nil1, nil2;
   nil1.reserve(0);
   nil2.squeeze();
   nil1.squeeze();
   nil2.reserve(0);
}

TEST(ByteArrayTest, testReseveExtended)
{
   std::list<ByteArray> data;
   prepare_prepend_data(data);
   std::list<ByteArray>::iterator begin = data.begin();
   std::list<ByteArray>::iterator end = data.end();
   while (begin != end) {
      ByteArray array = *begin;
      array.reserve(1024);
      ASSERT_EQ(array.capacity(), 1024);
      ASSERT_EQ(array, ByteArray("data"));
      array.squeeze();
      ASSERT_EQ(array, ByteArray("data"));
      ASSERT_EQ(array.capacity(), array.size());
      ++begin;
   }
}

TEST(ByteArrayTest, testMovablity)
{
   std::list<ByteArray> data;
   data.push_back(ByteArray("\x00\x00\x00\x00", 4));
   data.push_back(ByteArray("\x00\x00\x00\xff", 4));
   data.push_back(ByteArray(""));
   data.push_back(ByteArray());
   data.push_back(ByteArray(3, 's'));
   prepare_prepend_data(data);
   
   std::list<ByteArray>::iterator begin = data.begin();
   std::list<ByteArray>::iterator end = data.end();
   while (begin != end) {
      ByteArray array = *begin;
      array.reserve(1024);
      const int size = array.size();
      const bool isEmpty = array.isEmpty();
      const bool isNull = array.isNull();
      const int capacity = array.capacity();
      ByteArray memSpace;
      memSpace.~ByteArray();
      // move array -> memSpace
      std::memcpy(&memSpace, &array, sizeof(ByteArray));
      new (&array) ByteArray;
      ASSERT_EQ(memSpace.size(), size);
      ASSERT_EQ(memSpace.isEmpty(), isEmpty);
      ASSERT_EQ(memSpace.isNull(), isNull);
      ASSERT_EQ(memSpace.capacity(), capacity);
      PDK_UNUSED(memSpace.toLower());
      PDK_UNUSED(memSpace.toUpper());
      memSpace.prepend('a');
      memSpace.append("b", 1);
      memSpace.squeeze();
      memSpace.reserve(array.size() + 16);
      ByteArray copy(memSpace);
      // reinitialize base values
      const int newSize = size + 2;
      const bool newIsEmpty = false;
      const bool newIsNull = false;
      const int newCapacity = 16;
      // move back memSpace -> array
      array.~ByteArray();
      std::memcpy(&array, &memSpace, sizeof(ByteArray));
      new (&memSpace) ByteArray;
      ASSERT_EQ(array.size(), newSize);
      ASSERT_EQ(array.isEmpty(), newIsEmpty);
      ASSERT_EQ(array.isNull(), newIsNull);
      ASSERT_EQ(array.capacity(), newCapacity);
      ASSERT_TRUE(array.startsWith('a'));
      ASSERT_TRUE(array.endsWith('b'));
      
      ASSERT_EQ(copy.size(), newSize);
      ASSERT_EQ(copy.isEmpty(), newIsEmpty);
      ASSERT_EQ(copy.isNull(), newIsNull);
      ASSERT_EQ(copy.capacity(), newCapacity);
      ASSERT_TRUE(copy.startsWith('a'));
      ASSERT_TRUE(copy.endsWith('b'));
      
      array.squeeze();
      array.reserve(array.size() + 3);
      ASSERT_TRUE(true);
      ++begin;
   }
}

TEST(ByteArrayTest, testLiteral)
{
   ByteArray str(ByteArrayLiteral("abcd"));
   ASSERT_EQ(str.length(), 4);
   ASSERT_TRUE(str == "abcd");
   ASSERT_TRUE(str.getDataPtr()->m_ref.isStatic());
   ASSERT_TRUE(str.getDataPtr()->m_offset == sizeof(ByteArrayData));
   const char *s = str.getConstRawData();
   ByteArray str2 = str;
   ASSERT_TRUE(str2.getConstRawData() == s);
   // detach on non const access
   ASSERT_TRUE(str.getRawData() != s);
   ASSERT_TRUE(str2.getConstRawData() == s);
   ASSERT_TRUE(str2.getRawData() != s);
   ASSERT_TRUE(str2.getConstRawData() != s);
}

TEST(ByteArrayTest, testCompare)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray, int>>;
   DataType data;
   
   data.push_back(std::make_tuple(ByteArray(), ByteArray(), 0));
   data.push_back(std::make_tuple(ByteArray(), ByteArray(""), 0));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray(), 0));
   data.push_back(std::make_tuple(ByteArray(), ByteArray("abc"), -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray(), +1));
   data.push_back(std::make_tuple(ByteArray(""), ByteArray("abc"), -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray(""), +1));
   data.push_back(std::make_tuple(ByteArray::fromRawData("abc", 0), ByteArray("abc"), -1));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray::fromRawData("abc", 0), +1));
   
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("abc"), 0));
   data.push_back(std::make_tuple(ByteArray::fromRawData("abc", 3), ByteArray("abc"), 0));
   data.push_back(std::make_tuple(ByteArray::fromRawData("abcdef", 3), ByteArray("abc"), 0));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray::fromRawData("abc", 3), 0));
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray::fromRawData("abcdef", 3), 0));
   data.push_back(std::make_tuple(ByteArray("a\0bc", 4), ByteArray::fromRawData("a\0bc", 4), 0));
   data.push_back(std::make_tuple(ByteArray::fromRawData("a\0bcdef", 4), ByteArray("a\0bc", 4), 0));
   data.push_back(std::make_tuple(ByteArray("a\0bc", 4), ByteArray::fromRawData("a\0bcdef", 4), 0));
   
   data.push_back(std::make_tuple(ByteArray("000"), ByteArray("abc"), -1));
   data.push_back(std::make_tuple(ByteArray::fromRawData("00", 3), ByteArray("abc"), -1));
   data.push_back(std::make_tuple(ByteArray("000"), ByteArray::fromRawData("abc", 3), -1));
   data.push_back(std::make_tuple(ByteArray("abc", 3), ByteArray("abc", 4), -1));
   data.push_back(std::make_tuple(ByteArray::fromRawData("abc\0", 3), ByteArray("abc\0", 4), -1));
   data.push_back(std::make_tuple(ByteArray("a\0bc", 4), ByteArray("a\0bd", 4), -1));
   
   data.push_back(std::make_tuple(ByteArray("abc"), ByteArray("000"), 1));
   data.push_back(std::make_tuple(ByteArray("000"), ByteArray::fromRawData("00", 3), 1));
   data.push_back(std::make_tuple(ByteArray("abcd"), ByteArray::fromRawData("abc", 3), 1));
   data.push_back(std::make_tuple(ByteArray("a\0bc", 4), ByteArray("a\0bb", 4), 1));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray str1 = std::get<0>(item);
      ByteArray str2 = std::get<1>(item);
      int result = std::get<2>(item);
      const bool isEqual   = result == 0;
      const bool isLess    = result < 0;
      const bool isGreater = result > 0;
      
      // basic tests:
      ASSERT_EQ(str1 == str2, isEqual);
      ASSERT_EQ(str1 < str2, isLess);
      ASSERT_EQ(str1 > str2, isGreater);
      
      ASSERT_EQ(str1 <= str2, isLess | isEqual);
      ASSERT_EQ(str1 >= str2, isGreater | isEqual);
      ASSERT_EQ(str1 != str2, !isEqual);
      
      ASSERT_EQ(str2 == str1, isEqual);
      ASSERT_EQ(str2 < str1, isGreater);
      ASSERT_EQ(str2 > str1, isLess);
      
      ASSERT_EQ(str2 <= str1, isGreater | isEqual);
      ASSERT_EQ(str2 >= str1, isLess | isEqual);
      ASSERT_EQ(str2 != str1, !isEqual);
      
      if (isEqual) {
         // @TODO
         //ASSERT_TRUE(pdk::hash(str1) == pdk::hash(str2));
      }
      ++begin;
   }
}

TEST(ByteArrayTest, testCompareWithChar)
{
   using DataType = std::list<std::tuple<ByteArray, std::string, int>>;
   DataType data;
   
   data.push_back(std::make_tuple(ByteArray(), std::string(), 0));
   data.push_back(std::make_tuple(ByteArray(), std::string(""), 0));
   data.push_back(std::make_tuple(ByteArray(""), "abc", -1));
   data.push_back(std::make_tuple(ByteArray(), std::string(), 0));
   data.push_back(std::make_tuple(ByteArray(""), "", 0));
   data.push_back(std::make_tuple(ByteArray(""), std::string("abc"), -1));
   data.push_back(std::make_tuple(ByteArray::fromRawData("abc", 0), std::string(""), 0));
   data.push_back(std::make_tuple(ByteArray::fromRawData("abc", 0), std::string(""), 0));
   data.push_back(std::make_tuple(ByteArray::fromRawData("abc", 0), "abc", -1));
   
   data.push_back(std::make_tuple(ByteArray("abc"), std::string(), 1));
   data.push_back(std::make_tuple(ByteArray("abc"), "", 1));
   
   data.push_back(std::make_tuple(ByteArray("abc", 3), "abc", 0));
   data.push_back(std::make_tuple(ByteArray("abc"), "abc", 0));
   data.push_back(std::make_tuple( ByteArray::fromRawData("abcd", 3), "abc", 0));
   
   data.push_back(std::make_tuple(ByteArray("ab"), "abc", -1));
   data.push_back(std::make_tuple(ByteArray("abb"), "abc", -1));
   data.push_back(std::make_tuple(ByteArray::fromRawData("abc", 2), "abc", -1));
   data.push_back(std::make_tuple(ByteArray("", 1), "abc", -1));
   data.push_back(std::make_tuple(ByteArray::fromRawData("", 1), "abc", -1));
   data.push_back(std::make_tuple(ByteArray("a\0bc", 4), "a.bc", -1));
   
   data.push_back(std::make_tuple(ByteArray("ac"), "abc", 1));
   data.push_back(std::make_tuple(ByteArray("abd"), "abc", 1));
   data.push_back(std::make_tuple(ByteArray("abcd"), "abc", 1));
   data.push_back(std::make_tuple(ByteArray::fromRawData("abcd", 4), "abc", 1));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray str1 = std::get<0>(item);
      std::string string2 = std::get<1>(item);
      const char *str2 = string2.c_str();
      if (string2.empty())
         str2 = nullptr;
      int result = std::get<2>(item);
      const bool isEqual   = result == 0;
      const bool isLess    = result < 0;
      const bool isGreater = result > 0;
      
      // basic tests:
      ASSERT_EQ(str1 == str2, isEqual);
      ASSERT_EQ(str1 < str2, isLess);
      ASSERT_EQ(str1 > str2, isGreater);
      
      ASSERT_EQ(str1 <= str2, isLess | isEqual);
      ASSERT_EQ(str1 >= str2, isGreater | isEqual);
      ASSERT_EQ(str1 != str2, !isEqual);
      
      ASSERT_EQ(str2 == str1, isEqual);
      ASSERT_EQ(str2 < str1, isGreater);
      ASSERT_EQ(str2 > str1, isLess);
      
      ASSERT_EQ(str2 <= str1, isGreater | isEqual);
      ASSERT_EQ(str2 >= str1, isLess | isEqual);
      ASSERT_EQ(str2 != str1, !isEqual);
      
      if (isEqual) {
         // @TODO
         //ASSERT_TRUE(pdk::hash(str1) == pdk::hash(str2));
      }
      ++begin;
   }
}

TEST(ByteArrayTest, testToOrFromHex)
{
   using DataType = std::list<std::tuple<ByteArray, ByteArray, ByteArray>>;
   DataType data;
   data.push_back(std::make_tuple(ByteArray("libpdk is great!"), ByteArray("6c696270646b20697320677265617421"), 
                                  ByteArray("6c 69 62 70 64 6b 20 69 73 20 67 72 65 61 74 21")));
   data.push_back(std::make_tuple(ByteArray("libpdk is so great!"), ByteArray("6c696270646b20697320736f20677265617421"), 
                                  ByteArray("6c 69 62 70 64 6b 20 69 73 20 73 6f 20 67 72 65 61 74 21")));
   
   data.push_back(std::make_tuple(ByteArray(),  ByteArray(), ByteArray()));
   data.push_back(std::make_tuple(ByteArray(""),  ByteArray(""), ByteArray("")));
   data.push_back(std::make_tuple(ByteArray("\0", 1),  ByteArray("00"), ByteArray("0")));
   data.push_back(std::make_tuple(ByteArray("\xf", 1),  ByteArray("0f"), ByteArray("f")));
   data.push_back(std::make_tuple(ByteArray("\xaf", 1),  ByteArray("af"), ByteArray("xaf")));
   data.push_back(std::make_tuple(ByteArray("\xd\xde\xad\xc0\xde"),  
                                  ByteArray("0ddeadc0de"), ByteArray("ddeadc0de")));
   data.push_back(std::make_tuple(ByteArray("\xC\xde\xeC\xea\xee\xDe\xee\xee"),  
                                  ByteArray("0cdeeceaeedeeeee"), ByteArray("Code less. Create more. Deploy everywhere.")));
   data.push_back(std::make_tuple(ByteArray("\x1\x23"),  
                                  ByteArray("0123"), ByteArray("x123")));
   data.push_back(std::make_tuple(ByteArray("\x12\x34"),  
                                  ByteArray("1234"), ByteArray("x1234")));
   
   DataType::iterator begin = data.begin();
   DataType::iterator end = data.end();
   while (begin != end) {
      auto item = *begin;
      ByteArray str = std::get<0>(item);
      ByteArray hex = std::get<1>(item);
      ByteArray hexAlt1 = std::get<2>(item);
      
      {
         const ByteArray th = str.toHex();
         ASSERT_EQ(th.size(), hex.size());
         ASSERT_EQ(th, hex);
      }
      
      {
         const ByteArray th = ByteArray::fromHex(hex);
         ASSERT_EQ(th.size(), str.size());
         ASSERT_EQ(th, str);
      }
      
      ASSERT_EQ(ByteArray::fromHex(hexAlt1), str);
      ++begin;
   }
}
