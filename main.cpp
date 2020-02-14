//
// Created by pilarski on 14/02/2020.
//

#include <sstream>
#include <csv.hpp>

int main(){

  std::stringstream ss;
  ss <<
R"(test,zxcv,wer
asdf,234,456
234,345,678
234,345,890
123,234,123)";

  csv::csv_iterator<3> iterator(ss);
  for(auto&[first, second, last] : iterator){
    std::cout << first << " | " << second << " | " << last << std::endl;
  }

}