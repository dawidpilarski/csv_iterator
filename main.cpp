//
// Created by pilarski on 14/02/2020.
//

#include <sstream>
#include <csv.hpp>

int main(){

  std::stringstream ss;
  ss <<
R"(test
asdf
234
234
123)";

  csv::csv_iterator<1> iterator(ss);
  for(auto&[first] : iterator){
    std::cout << first << " | " << " | " << std::endl;
  }

}