#include "precompile_header.h"

#include "TextCompoundIterator.h"
#include "DirectedStreambuf.h"

using namespace TextCompoundIteratorState;
using namespace std;

void fillCompoundRecord(CompoundRecord& rec, const string& str)
{
   vector<string> partsOfStringCompoundRecord;
   boost::split(partsOfStringCompoundRecord, str, boost::is_any_of(","));
   
   rec._compoundId = partsOfStringCompoundRecord[0];
   partsOfStringCompoundRecord.erase(partsOfStringCompoundRecord.begin());
   
   rec._features.clear();
   BOOST_FOREACH(const string& value, partsOfStringCompoundRecord)
   {
      rec._features.push_back(boost::lexical_cast<uint32_t>(value));
   }
}

TextCompoundIterator::TextCompoundIterator()
{
   set_State(Unknown);
}

TextCompoundIterator::TextCompoundIterator(const TextCompoundIterator& other)
{
   switch (other.get_State())
   {
   case Finish:
      set_Path(other.get_Path());
      set_State(Finish);
      break;
   case Begin:
      set_Path(other.get_Path());
      set_State(Begin);
      validate();
      break;
   case Proceed:
      throw exception();
   case Unknown:
      throw exception();
   default:
      throw exception();
   }
}

TextCompoundIterator::TextCompoundIterator(const boost::filesystem::path& path, TextCompoundIteratorStateEnum state)
{
   set_Path(path);
   set_State(state);
   validate();
}

TextCompoundIterator::~TextCompoundIterator()
{
   /*if (_inputFileStream.is_open())
   {
      _inputFileStream.close();
   }*/
}

void TextCompoundIterator::increment()
{
   string tmp;
   switch (get_State())
   {
   case Begin:
      set_State(Proceed);
   case Proceed:
      read_CurrentCompoundRecord();
      break;
   case Finish:
      throw exception();
   case Unknown:
      throw exception();
   default:
      throw exception();
   }
}

bool TextCompoundIterator::equal(const TextCompoundIterator& other) const
{
   switch (other.get_State())
   {
   case Begin:
   case Finish:
      return (get_State() == other.get_State());
   case Proceed:
      return false;
   case Unknown:
      throw exception();
   default:
      throw exception();
   }
}

CompoundRecord& TextCompoundIterator::dereference() const
{
   return _currentCompoundRecord;
}

void TextCompoundIterator::validate()
{
   switch (get_State())
   {
   case Finish:
   case Proceed:
      break;
   case Begin:
      if (!_file.is_open())
      {
         boost::iostreams::mapped_file_params params(get_Path().generic_string());
         params.mode = ios_base::in;
         _file.open(params);
         _istreamPtr = DirectedIstreamPtr(new DirectedIstream(_file.data(), _file.size()));
      }
      break;
   case Unknown:
      throw exception();
   default:
      throw exception();
   }
}

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

void TextCompoundIterator::read_CurrentCompoundRecord()
{
   boost::iostreams::mapped_file_source m("path");
   //m.is_open()
   
   
   boost::iostreams::stream < boost::iostreams::mapped_file_source > str(m);
   //m.open();

   string s;
   getline(str, s);

   


   /*string tmp;
   getline(_inputFileStream, tmp);
   if (tmp == "")
   {
      _inputFileStream.close();
      set_State(Finish);
   }
   else
   {
      fillCompoundRecord(_currentCompoundRecord, tmp);
   }*/
}

void TextCompoundIterator::set_Path(const boost::filesystem::path& path)
{
   _path = path;
}

void TextCompoundIterator::set_State(TextCompoundIteratorStateEnum state)
{
   _state = state;
}

const boost::filesystem::path& TextCompoundIterator::get_Path() const
{
   return _path;
}

const TextCompoundIteratorStateEnum TextCompoundIterator::get_State() const
{
   return _state;
}