# csv_iterator
C++ lightwight, header only csv_iterator class to read csv files. 


## Usage

### Installation
csv_iterator is a header only library, so installation is as easy as copying the csv/csv.hpp to your project include
directory and start using it.

### API

csv_iterator assumes you have knowledge about the structure of the csv file you want to process. Meaning, that its 
suitable to semantically process content of the file, but its out of use to make processing on any random csv file
you want.

csv_iterator also operates in two modes. One assumes, that you can guarantee, that syntax of your csv content is 
correct, and second one doesn't make such assumption, but additional checks performed on the csv content makes
the iterator slower. The "checkful" mode is set by the second non type template argument of the csv_iterator template.

Iterator fulfills `LegacyInputIterator` concept.

#### creation and basic usage

```c++
csv_iterator<2, should_check_correctness> it(stream, delimiter); //#1
for(auto&[row1, row2] : it){ //#2
  std::cout << row1 << " || " << row2 << std::endl;
}
``` 

on line #1 we are creating an iterator over the stream `stream` with csv content consisting of two rows.
Whether or not file correctness is checked is set by `should_check_correctness` boolean flag. In case of 
error in the content, when correctness is being checked, `csv_error` inheriting from `runtime_error` exception is being thrown.
Otherwise if error is encountered, the behavior is undefined.

On line #2 we are iterating over the csv content using previously created iterator. dereferencing iteratior
returns `std::array<std::string_view, number_of_rows>` type by reference. The result of processing line is cached
inside the csv_iterator type.

Default delimiter for the iterator is a comma `,`, but custom delimiters can be provided.

# License

BSD 3-Clause License - details in LICENSE file