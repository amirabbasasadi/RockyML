/*################################################################################
  ##
  ##   Copyright (C) 2011-2022 Keith O'Hara
  ##
  ##   This file is part of the StatsLib C++ library.
  ##
  ##   Licensed under the Apache License, Version 2.0 (the "License");
  ##   you may not use this file except in compliance with the License.
  ##   You may obtain a copy of the License at
  ##
  ##       http://www.apache.org/licenses/LICENSE-2.0
  ##
  ##   Unless required by applicable law or agreed to in writing, software
  ##   distributed under the License is distributed on an "AS IS" BASIS,
  ##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  ##   See the License for the specific language governing permissions and
  ##   limitations under the License.
  ##
  ################################################################################*/

/*
 * Sample from a Binomial distribution
 */

//
// scalar output

template<typename T>
statslib_inline
return_t<T>
rbinom(const llint_t n_trials_par, const T prob_par, rand_engine_t& engine)
{
    if (!internal::binom_sanity_check(n_trials_par,prob_par)) {
        return STLIM<T>::quiet_NaN();
    }

    //

    return_t<T> ret = return_t<T>(0);

    for (llint_t i=llint_t(0); i < n_trials_par; ++i)
    {
        ret += rbern(prob_par,engine);
    }

    return ret;
}

template<typename T>
statslib_inline
return_t<T>
rbinom(const llint_t n_trials_par, const T prob_par, const ullint_t seed_val)
{
    rand_engine_t engine(seed_val);
    return rbinom(n_trials_par,prob_par,engine);
}

//
// matrix output

namespace internal
{

#ifdef STATS_ENABLE_INTERNAL_VEC_FEATURES
template<typename T1, typename rT>
statslib_inline
void
rbinom_vec(const llint_t n_trials_par, const T1 prob_par, rT* __stats_pointer_settings__ vals_out, const ullint_t num_elem)
{
    RAND_DIST_FN_VEC(rbinom,vals_out,num_elem,n_trials_par,prob_par);
}
#endif

#ifdef STATS_ENABLE_STDVEC_WRAPPERS
template<typename eT, typename T1>
statslib_inline
void
rbinom_mat_check(std::vector<eT>& X, const llint_t n_trials_par, const T1 prob_par)
{
    STDVEC_RAND_DIST_FN(rbinom,n_trials_par,prob_par);
}
#endif

#ifdef STATS_ENABLE_MATRIX_FEATURES
template<typename mT, typename T1>
statslib_inline
void
rbinom_mat_check(mT& X, const llint_t n_trials_par, const T1 prob_par)
{
    MAIN_MAT_RAND_DIST_FN(rbinom,n_trials_par,prob_par);
}
#endif

}

#ifdef STATS_ENABLE_INTERNAL_VEC_FEATURES
template<typename mT, typename T1>
statslib_inline
mT
rbinom(const ullint_t n, const ullint_t k, const llint_t n_trials_par, const T1 prob_par)
{
    GEN_MAT_RAND_FN(rbinom_mat_check,n_trials_par,prob_par);
}
#endif
