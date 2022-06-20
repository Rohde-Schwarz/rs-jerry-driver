#include <cinttypes>
#include <fstream>
#include <vector>

class HrzrTestBinaryHelper {
public:
   static std::vector<uint64_t> fillInputFromFile(const std::string &fileName)
   {
      std::vector<uint64_t> testInput;
      std::fstream fs(fileName, std::fstream::in | std::fstream::binary);

      for (;;)
      {
         uint64_t nextValue = 0;
         fs.read(reinterpret_cast<char *>(&nextValue), sizeof(uint64_t));

         if (fs.eof())
            break;
         else if (!fs.good())
            throw std::runtime_error("Test data could not be read from file");
         testInput.push_back(nextValue);
      }

      fs.close();
      return testInput;
   }
};