#include "common/definitions.h"
#include "common/exceptions.h"
#include "dpdkSource/hrzrParser.h"

#include <gtest/gtest.h>

#include <fstream>

const int NUMBER_OF_SAMPLES = 10;

static std::vector<std::string> fillInputFromFile(const std::string &fileName)
{
   std::vector<std::string> rawInput;
   std::fstream fs(fileName, std::fstream::in);
   for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
   {
      if (!fs.good() || fs.eof())
      {
         throw std::runtime_error("Test data could not be read from file");
      }
      std::string line;
      getline(fs, line);
      const auto index_of_seperator = line.find(',');
      if (index_of_seperator != std::string::npos)
      {
         const auto first_substring = line.substr(1, index_of_seperator);
         rawInput.push_back(first_substring);
         const auto second_substring = line.substr(index_of_seperator + 1, line.size() - 2);
         rawInput.push_back(second_substring);
      }
   }
   fs.close();
   return rawInput;
}

static std::vector<std::complex<float>> convertInputToComplexFloat(const std::string &fileName)
{
   std::vector<std::complex<float>> input(NUMBER_OF_SAMPLES);
   const auto rawInput = fillInputFromFile(fileName);
   for (auto i = 0UL; i < input.size(); ++i)
   {
      const auto &first_number  = std::stof(rawInput[2 * i]);
      const auto &second_number = std::stof(rawInput[2 * i + 1]);
      input[i]                  = {first_number, second_number};
   }
   return input;
}

static std::vector<int16_t> convertInputToShort(const std::string &fileName)
{
   std::vector<int16_t> input;
   const auto rawInput = fillInputFromFile(fileName);
   for (const auto &value : rawInput) { input.push_back(std::stoi(value)); }
   return input;
}

TEST(HrzrParserTest, checksNorm)
{
   EXPECT_THROW(HrzrParser(0), InvalidValueError);
}

TEST(HrzrParserTest, processesOnePacket)
{
   std::vector<int16_t> payload(definitions::SAMPLES_PER_PACKET * 2);
   std::vector<std::complex<float>> samples(definitions::SAMPLES_PER_PACKET);
   HrzrParser parser(1U);
   const auto processedSamples = parser.parsePayloadSamples(0, &payload[0], &samples[0]);
   EXPECT_FLOAT_EQ(definitions::SAMPLES_PER_PACKET, processedSamples);
}

TEST(HrzrParserTest, appliesNormToPackets)
{
   auto inputSamples = convertInputToShort("floatsamples.in");
   inputSamples.resize(definitions::SAMPLES_PER_PACKET * 2);
   std::vector<std::complex<float>> outputSamples(definitions::SAMPLES_PER_PACKET);

   const auto testNorm = 10U;
   HrzrParser parser(testNorm);
   parser.parsePayloadSamples(0, &inputSamples[0], &outputSamples[0]);

   const auto referenceSamples = convertInputToComplexFloat("floatsamples.norm");
   for (auto i = 0UL; i < referenceSamples.size(); ++i) EXPECT_EQ(referenceSamples.at(i), outputSamples.at(i));
}
