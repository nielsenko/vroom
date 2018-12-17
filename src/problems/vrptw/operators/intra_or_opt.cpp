/*

This file is part of VROOM.

Copyright (c) 2015-2018, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "problems/vrptw/operators/intra_or_opt.h"

vrptwIntraOrOpt::vrptwIntraOrOpt(const Input& input,
                                 const SolutionState& sol_state,
                                 TWRoute& tw_s_route,
                                 Index s_vehicle,
                                 Index s_rank,
                                 Index t_rank)
  : CVRPIntraOrOpt(input,
                   sol_state,
                   static_cast<RawRoute&>(tw_s_route),
                   s_vehicle,
                   s_rank,
                   t_rank),
    _tw_s_route(tw_s_route),
    _is_normal_valid(false),
    _is_reverse_valid(false),
    _moved_jobs((s_rank < t_rank) ? t_rank - s_rank + 2 : s_rank - t_rank + 2),
    _first_rank(std::min(s_rank, t_rank)),
    _last_rank(std::max(s_rank, t_rank) + 2) {
  if (t_rank < s_rank) {
    _s_edge_first = 0;
    _s_edge_last = 1;

    std::copy(s_route.begin() + t_rank,
              s_route.begin() + s_rank,
              _moved_jobs.begin() + 2);
  } else {
    _s_edge_first = _moved_jobs.size() - 2;
    _s_edge_last = _moved_jobs.size() - 1;

    std::copy(s_route.begin() + s_rank + 2,
              s_route.begin() + t_rank + 2,
              _moved_jobs.begin());
  }

  _moved_jobs[_s_edge_first] = s_route[s_rank];
  _moved_jobs[_s_edge_last] = s_route[s_rank + 1];
}

void vrptwIntraOrOpt::compute_gain() {
  CVRPIntraOrOpt::compute_gain();
  assert(_is_normal_valid or _is_reverse_valid);

  if (reverse_s_edge) {
    if (!_is_reverse_valid) {
      // Biggest potential gain is obtained when reversing edge, but
      // this does not match TW constraints, so update gain and edge
      // direction to not reverse.
      stored_gain = normal_stored_gain;
      reverse_s_edge = false;
    }
  } else {
    if (!_is_normal_valid) {
      // Biggest potential gain is obtained when not reversing edge,
      // but this does not match TW constraints, so update gain and
      // edge direction to reverse.
      stored_gain = reversed_stored_gain;
      reverse_s_edge = true;
    }
  }
}

bool vrptwIntraOrOpt::is_valid() {
  _is_normal_valid = _tw_s_route.is_valid_addition_for_tw(_input,
                                                          _moved_jobs.begin(),
                                                          _moved_jobs.end(),
                                                          _first_rank,
                                                          _last_rank);

  std::swap(_moved_jobs[_s_edge_first], _moved_jobs[_s_edge_last]);

  _is_reverse_valid = _tw_s_route.is_valid_addition_for_tw(_input,
                                                           _moved_jobs.begin(),
                                                           _moved_jobs.end(),
                                                           _first_rank,
                                                           _last_rank);

  // Reset to initial situation before potential application.
  std::swap(_moved_jobs[_s_edge_first], _moved_jobs[_s_edge_last]);

  return _is_normal_valid or _is_reverse_valid;
}

void vrptwIntraOrOpt::apply() {
  if (reverse_s_edge) {
    std::swap(_moved_jobs[_s_edge_first], _moved_jobs[_s_edge_last]);
  }

  _tw_s_route.replace(_input,
                      _moved_jobs.begin(),
                      _moved_jobs.end(),
                      _first_rank,
                      _last_rank);
}

std::vector<Index> vrptwIntraOrOpt::addition_candidates() const {
  return {s_vehicle};
}