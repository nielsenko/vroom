/*
VROOM (Vehicle Routing Open-source Optimization Machine)
Copyright (C) 2015, Julien Coupey

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MUNKRES_H
#define MUNKRES_H
#include <map>
#include <limits>
#include <set>
#include "../structures/matrix.h"

template <class T>
std::map<unsigned, unsigned> minimum_weight_perfect_matching(const matrix<T>& m){
  // std::cout << "Entering mwpm" << std::endl;

  // Trivial initial labeling.
  std::map<unsigned, T> labeling_x;
  std::map<unsigned, T> labeling_y;
  for(unsigned i = 0; i < m.size(); ++i){
    labeling_y.emplace(i, 0);
    T min_weight = std::numeric_limits<T>::max();
    for(unsigned j = 0; j < m.size(); ++j){
      if(m(i, j) < min_weight){
        min_weight = m(i, j);
      }
    }
    labeling_x.emplace(i, min_weight);
  }

  // Initial empty matching.
  std::map<unsigned, unsigned> matching_xy;
  std::map<unsigned, unsigned> matching_yx;

  // Alternating tree.
  std::map<unsigned, unsigned> alternating_tree;

  while(matching_xy.size() < m.size()){
    // Step 1.

    // std::cout << "Entering step 1.\n";

    // std::cout << "Labeling x\n";
    // for(unsigned x = 0; x < m.size(); ++x){
    //   std::cout << x << ": " << labeling_x.at(x) << std::endl;
    // }
    // std::cout << "Labeling y\n";
    // for(unsigned y = 0; y < m.size(); ++y){
    //   std::cout << y << ": " << labeling_y.at(y) << std::endl;
    // }

    alternating_tree.clear();
    std::set<unsigned> S;
    std::set<unsigned> T_set;
    
    // Finding any unmatched x.
    unsigned unmatched_x = 0;
    while(matching_xy.find(unmatched_x) != matching_xy.end()){
      ++unmatched_x;
    }
    // std::cout << "unmatched_x: " << unmatched_x << std::endl;
    S.insert(unmatched_x);

    // Saving relevant neighbors in equality graph in alternating_tree
    // and initializing slacks.
    std::map<unsigned, T> slack;
    for(unsigned y = 0; y < m.size(); ++y){
      if(labeling_x.at(unmatched_x) + labeling_y.at(y) == m(unmatched_x, y)){
        alternating_tree.emplace(y, unmatched_x);
      }
      slack.emplace(y,
                    m(unmatched_x, y) - labeling_x.at(unmatched_x) - labeling_y.at(y)
                    );
    }

    // std::cout << "Slack values\n";
    // for(auto s = slack.cbegin(); s != slack.cend(); ++s){
    //   std::cout << s->first << ": " << s->second << std::endl;
    // }
    
    bool augmented_path = false;

    while(!augmented_path){
      // Test if neighbors of S in equality graph equals T_set or not
      // (note that T_set is included in S neighbors).
      if(alternating_tree.size() == T_set.size()){
        // Step 2.
        // std::cout << "Entering step 2.\n";

        // std::cout << "S: ";
        // for(auto s = S.cbegin(); s != S.cend(); ++s){
        //   std::cout << *s << " ; ";
        // }
        // std::cout << std::endl;
        // std::cout << "T_set: ";
        // for(auto s = T_set.cbegin(); s != T_set.cend(); ++s){
        //   std::cout << *s << " ; ";
        // }
        // std::cout << std::endl;

        T alpha = std::numeric_limits<T>::max();
        for(unsigned y = 0; y < m.size(); ++y){
          // Computing alpha, the minimum of slack values over
          // complement of T_set.
          if(T_set.find(y) == T_set.end()){
            T current_slack = slack.at(y);
            // std::cout << "slack for " << y << ": " << slack.at(y) << std::endl;
            if(current_slack < alpha){
              alpha = current_slack;
            }
          }
        }

        // Update labelings

        // std::cout << "Updating with alpha = " << alpha << std::endl;

        for(auto x = S.cbegin(); x != S.cend(); ++x){
          labeling_x.at(*x) = labeling_x.at(*x) + alpha;
        }
        for(auto y = T_set.cbegin(); y != T_set.cend(); ++y){
          labeling_y.at(*y) = labeling_y.at(*y) - alpha;
        }

        // Updating relevant neighbors in new equality graph and
        // updating slacks.
        for(unsigned y = 0; y < m.size(); ++y){
          if(T_set.find(y) == T_set.end()){
            slack.at(y) = slack.at(y) - alpha;
            
            for(auto x = S.cbegin(); x != S.cend(); ++x){
              if(labeling_x.at(*x) + labeling_y.at(y) == m(*x, y)){
                if(alternating_tree.find(y) == alternating_tree.end()){
                  alternating_tree.emplace(y, *x);
                }
              }
            }
          }
        }

        // std::cout << "Labeling x\n";
        // for(unsigned x = 0; x < m.size(); ++x){
        //   std::cout << x << ": " << labeling_x.at(x) << std::endl;
        // }
        // std::cout << "Labeling y\n";
        // for(unsigned y = 0; y < m.size(); ++y){
        //   std::cout << y << ": " << labeling_y.at(y) << std::endl;
        // }
        // std::cout << "Slack values\n";
        // for(auto s = slack.cbegin(); s != slack.cend(); ++s){
        //   std::cout << s->first << ": " << s->second << std::endl;
        // }

      }

      // Step 3.
      // std::cout << "Entering step 3.\n";

      // std::cout << "Alternating tree: \n";
      // for(auto edge = alternating_tree.cbegin();
      //     edge != alternating_tree.cend();
      //     ++edge){
      //   std::cout << edge->first << "->" << edge->second << std::endl;
      // }

      unsigned chosen_y;        // First y in equality neighbors not
                                // in T_set.
      for(auto edge = alternating_tree.cbegin();
          edge != alternating_tree.cend();
          ++edge){
        if(T_set.find(edge->first) == T_set.end()){
          // MUST happen before endge reaches the end of
          // alternating_tree.
          chosen_y = edge->first;
          // std::cout << "chosen y: " << chosen_y << std::endl;
          break;
        }
      }

      auto matching_y = matching_yx.find(chosen_y);
      if(matching_y != matching_yx.end()){
        // Chosen y is actually matched in M, update S and T_set and
        // proceed to step 2.
        unsigned matched_x = matching_y->second;

        // std::cout << "Matched with " << matched_x << std::endl;
        S.insert(matched_x);
        T_set.insert(chosen_y);

        // Updating slacks.
        for(unsigned y = 0; y < m.size(); ++y){
          T current_value = slack.at(y);
          T new_value
            = m(matched_x, y) - labeling_x.at(matched_x) - labeling_y.at(y);
          // std::cout << "In update slacks, current_value: "
          //           << current_value
          //           << " and new value: "
          //           << new_value
          //           << std::endl;
          if(new_value < current_value){
            slack.at(y) = new_value;
          }
        }
      }
      else{
        // std::cout << "Unmatched" << std::endl;

        // Find larger matching using M-alternating path. The path is
        // described at each step by:
        // 
        // chosen_y -- chosen_x -- next_y -- [...] -- unmatched_x
        //
        // where (chosen_x, next_y) is already in matching and should
        // be removed and (chosen_x, chosen_y) is to be added.
        
        unsigned current_y = chosen_y;
        unsigned current_x = alternating_tree.at(current_y);

        while(current_x != unmatched_x){
          unsigned next_y = matching_xy.at(current_x);

          // Remove alternating edge from current matching.
          matching_xy.erase(matching_xy.find(current_x));
          matching_yx.erase(matching_yx.find(next_y));
          
          // Add edge from alternating tree to matching.
          matching_xy.emplace(current_x, current_y);
          matching_yx.emplace(current_y, current_x);
          // std::cout << current_y << " - " << current_x << " - ";

          current_y = next_y;
          current_x = alternating_tree.at(current_y);
        }
        // Adding last edge from alternating tree.
        matching_xy.emplace(current_x, current_y);
        matching_yx.emplace(current_y, current_x);
        // std::cout << current_y << " - " << current_x << " - ";

        // std::cout << std::endl;
        // Back to step 1.
        augmented_path = true;
        
        // std::cout << "Matching so far: \n";
        // for(auto edge = matching_xy.cbegin(); edge != matching_xy.cend(); ++edge){
        //   std::cout << edge->first << "->" << edge->second << std::endl;
        // }
      }
    }
    // std::cout << "Matching size: " << matching_xy.size() << std::endl;
  }
  // std::cout << "Matching size before return: " << matching_xy.size() << std::endl;
  return matching_xy;
}

#endif