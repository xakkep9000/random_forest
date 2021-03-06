#pragma once

#include "precompile_header.h"

#include "TemplateParser.h"
#include "CompoundRecord.h"
#include "TargetRecord.h"
#include "PredRecord.h"

template<>
struct RawRecordParser<CompoundRecordOptional>
{
   static void parse(CompoundRecordOptional& opRec, const std::string& str)
   {
      CompoundRecord rec;

      opRec.reset();

      std::vector<std::string> partsOfStringCompoundRecord;
      boost::split(partsOfStringCompoundRecord, str, boost::is_any_of(","));
      if (partsOfStringCompoundRecord.size() < 2)
         return;

      try
      {
         rec._compoundId = partsOfStringCompoundRecord[0];
      }
      catch(const CompoundIdException&)
      {
         return;
      }
   
   
      //rec._features.resize(partsOfStringCompoundRecord.size() - 1);
      bool fail = false;
      for (std::vector<std::string>::iterator itr = partsOfStringCompoundRecord.begin() + 1; itr != partsOfStringCompoundRecord.end(); ++itr)
      {
         try
         {
            rec._features.push_back(boost::lexical_cast<uint32_t>(*itr));
         }
         catch(const boost::bad_lexical_cast &)
         {
            fail = true;
            break;
         }
      }
      if (!fail)
      {
         opRec = rec;
      }
   }
};

template<>
struct FormatHintRawRecordParser<FileFormat::EndOfLineSeparator, CompoundRecordOptional>
{
   static void parse(CompoundRecordOptional& opRec, const std::string& str)
   {
      RawRecordParser<CompoundRecordOptional>::parse(opRec, str);
   }
};

template<>
struct RawRecordParser<TargetRecordOptional>
{
   static void parse(TargetRecordOptional& opRec, const std::string& str)
   {
      TargetRecord rec;
      
      opRec.reset();
      
      std::vector<std::string> partsOfStringTargetRecord;
      boost::split(partsOfStringTargetRecord, str, boost::is_any_of(" "));
      if (partsOfStringTargetRecord.size() < 2)
         return;
      
      try
      {
         rec._compoundId = partsOfStringTargetRecord[0];
         rec._target = (uint8_t)(boost::lexical_cast<int>(partsOfStringTargetRecord[1]));
      }
      catch(const CompoundIdException&)
      {
         return;
      }
      catch(const boost::bad_lexical_cast &)
      {
         return;
      }
      
      opRec = rec;
   }
};

template<>
struct FormatHintRawRecordParser<FileFormat::EndOfLineSeparator, TargetRecordOptional>
{
   static void parse(TargetRecordOptional& opRec, const std::string& str)
   {
      RawRecordParser<TargetRecordOptional>::parse(opRec, str);
   }
};

template<>
struct RawRecordParser<PredRecordOptional>
{
   static void parse(PredRecordOptional& opRec, const std::string& str)
   {
      PredRecord rec;
      
      opRec.reset();
      
      std::vector<std::string> partsOfStringTargetRecord;
      boost::split(partsOfStringTargetRecord, str, boost::is_any_of(" "));
      if (partsOfStringTargetRecord.size() < 1)
         return;
      
      try
      {
         rec._compoundId = partsOfStringTargetRecord[0];
      }
      catch(const CompoundIdException&)
      {
         return;
      }
      catch(const boost::bad_lexical_cast &)
      {
         return;
      }
      
      opRec = rec;
   }
};

template<>
struct FormatHintRawRecordParser<FileFormat::EndOfLineSeparator, PredRecordOptional>
{
   static void parse(PredRecordOptional& opRec, const std::string& str)
   {
      RawRecordParser<PredRecordOptional>::parse(opRec, str);
   }
};