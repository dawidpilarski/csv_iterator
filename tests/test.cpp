/*
 * Copyright 2020 dawid.pilarski@panicsoftware.com
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <csv/csv.hpp>

#include <algorithm>

using namespace csv;

TEST_CASE("csv_iterator meets input iterator criteria"){
  std::stringstream ss;
  ss <<
R"(header,only,stuff,etc
1,2,3,4
4,3,2,1
2,3,1,4
3,1,2,12412312341
,12,3,
)";

  std::stringstream ss2;
  ss2 <<
R"(not|much|to|say
1|2|3|4
4|3|2|1
)";

  csv_iterator<4> it1(ss);
  csv_iterator<4> end_it;
  csv_iterator<4> it2(ss2, '|');

  SECTION("csv_iterator meets EqualityComparable"){
    SECTION("self comparisons shall always return true"){
      REQUIRE(it1 == it1);
      REQUIRE(it2 == it2);
      REQUIRE(end_it == end_it);

      REQUIRE(!(it1 != it1));
      REQUIRE(!(it2 != it2));
      REQUIRE(!(end_it != end_it));
    }

    SECTION("iterators to different streams shall not compare equal"){
      REQUIRE(it1 != it2);
      REQUIRE(it2 != it1);
      REQUIRE(it1 != end_it);
      REQUIRE(end_it != it1);
      REQUIRE(it2 != end_it);
      REQUIRE(end_it != it2);

      REQUIRE(!(it1 == it2));
      REQUIRE(!(it2 == it1));
      REQUIRE(!(it1 == end_it));
      REQUIRE(!(end_it == it1));
      REQUIRE(!(it2 == end_it));
      REQUIRE(!(end_it == it2));
    }

    SECTION("Equality should be transitive"){
      csv_iterator<4> end_it2;
      csv_iterator<4> end_it3;

      REQUIRE(end_it == end_it2);
      REQUIRE(end_it == end_it3);
      REQUIRE(end_it2 == end_it3);
    }
  }

  SECTION("csv_iterator meets LegacyIteratorCriteria"){
    SECTION("csv_iterator meets CopyConstructible"){
      SECTION("csv_iterator meets MoveConstructible"){
        csv_iterator<4> lhs = std::move(it1);
        REQUIRE(lhs != end_it);
        // crash tests
        *lhs;
        ++lhs;

        csv_iterator<4> lhs2(std::move(it2));
        REQUIRE(lhs != end_it);
        //crash tests
        *lhs2;
        ++lhs2;

        //no need to restore original values, because move behaves like copy
      }

      csv_iterator<4> cpy1 = static_cast<const csv_iterator<4>&>(it1);
      REQUIRE(cpy1 == it1);
      REQUIRE(cpy1 != end_it);

      csv_iterator<4> cpy2(static_cast<const csv_iterator<4>&>(it2));
      REQUIRE(cpy2 == it2);
      REQUIRE(cpy2 != end_it);
    }

    SECTION("csv_iterator meets CopyAssignable"){
      SECTION("csv_iterator meets MoveAssignable"){

      }
    }

    SECTION("csv_iterator meets Destructible"){
      REQUIRE(std::is_nothrow_destructible_v<csv_iterator<3>>);
    }

    SECTION("csv_iterator meets Swappable"){
      using std::swap;
      auto it1cp = it1;
      auto it2cp = it2;
      swap(it1, it2);
      REQUIRE(it2 == it1cp);
      REQUIRE(it1 == it2cp);

      //todo finish
    }

    //todo finish
  }
}

TEST_CASE("Compatibility with standard algorithms test"){
  SECTION("std::distance test"){

  }

  SECTION("std::next test"){

  }
}
