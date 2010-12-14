/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2008, Rice University
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Rice University nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Ioan Sucan */

#ifndef OMPL_GEOMETRIC_PLANNERS_RRT_OPTRRT_
#define OMPL_GEOMETRIC_PLANNERS_RRT_OPTRRT_

#include "ompl/geometric/planners/PlannerIncludes.h"
#include "ompl/datastructures/NearestNeighbors.h"

namespace ompl
{

    namespace geometric
    {

        /**
           @anchor gOptRRT

           @par Short description
           RRT version that rewires the exploration tree as it is
           being grown. This allows finding optimal solutions, given
           enough time. The notion of optimality is with respect to
           the distance function defined on the manifold we are
           operating on. See ompl::base::Goal::setMaximumPathLength()
           for how to set the maximally allowed path length to reach
           the goal. If a solution path that is shorter than
           ompl::base::Goal::getMaximumPathLength() is found, the
           algorithm terminates before the elapsed time.  The
           algorithm is fairly sensitive to setMaxBallRadius() and
           setBallRadiusConstant(), so make sure to experiment with
           different values.

           @par External documentation
           S. Karaman and E. Frazzoli, Incremental Sampling-based
           Algorithms for Optimal Motion Planning, Robotics: Science
           and Systems (RSS) Conference, 2010.
           <a href="http://arxiv.org/abs/1005.0416">http://arxiv.org/abs/1005.0416</a>

        */

        /** \brief Rapidly-exploring Random Trees with Optimization */
        class OptRRT : public base::Planner
        {
        public:

            OptRRT(const base::SpaceInformationPtr &si) : base::Planner(si, "OptRRT")
            {
                type_ = base::PLAN_TO_GOAL_ANY;

                goalBias_ = 0.05;
                maxDistance_ = 0.0;
                ballRadiusMax_ = 0.0;
                ballRadiusConst_ = 1.0;
            }

            virtual ~OptRRT(void)
            {
                freeMemory();
            }

            virtual void getPlannerData(base::PlannerData &data) const;

            virtual bool solve(const base::PlannerTerminationCondition &ptc);

            virtual void clear(void);

            /** \brief Set the goal bias

                In the process of randomly selecting states in
                the state space to attempt to go towards, the
                algorithm may in fact choose the actual goal state, if
                it knows it, with some probability. This probability
                is a real number between 0.0 and 1.0; its value should
                usually be around 0.05 and should not be too large. It
                is probably a good idea to use the default value. */
            void setGoalBias(double goalBias)
            {
                goalBias_ = goalBias;
            }

            /** \brief Get the goal bias the planner is using */
            double getGoalBias(void) const
            {
                return goalBias_;
            }

            /** \brief Set the range the planner is supposed to use.

                This parameter greatly influences the runtime of the
                algorithm. It represents the maximum length of a
                motion to be added in the tree of motions. */
            void setRange(double distance)
            {
                maxDistance_ = distance;
            }

            /** \brief Get the range the planner is using */
            double getRange(void) const
            {
                return maxDistance_;
            }

            /** \brief When the planner attempts to rewire the tree,
                it does so by looking at some of the neighbors within
                a computed radius. The computation of that radius
                depends on the multiplicative factor set here.*/
            void setBallRadiusConstant(double ballRadiusConstant)
            {
                ballRadiusConst_ = ballRadiusConstant;
            }

            /** \brief Get the multiplicative factor used in the
                computation of the radius whithin which tree rewiring
                is done. */
            double getBallRadiusConstant(void) const
            {
                return ballRadiusConst_;
            }

            /** \brief When the planner attempts to rewire the tree,
                it does so by looking at some of the neighbors within
                a computed radius. That radius is bounded by the value
                set here.*/
            void setMaxBallRadius(double maxBallRadius)
            {
                ballRadiusMax_ = maxBallRadius;
            }

            /** \brief Get the maximum radius the planner uses in the
                tree rewiring step */
            double getMaxBallRadius(void) const
            {
                return ballRadiusMax_;
            }

            /** \brief Set a different nearest neighbors datastructure */
            template<template<typename T> class NN>
            void setNearestNeighbors(void)
            {
                nn_.reset(new NN<Motion*>());
            }

            virtual void setup(void);

        protected:


            /** \brief Representation of a motion

                This only contains pointers to parent motions as we
                only need to go backwards in the tree. */
            class Motion
            {
            public:

                Motion(void) : state(NULL), parent(NULL), cost(0.0)
                {
                }

                Motion(const base::SpaceInformationPtr &si) : state(si->allocState()), parent(NULL), cost(0.0)
                {
                }

                ~Motion(void)
                {
                }

                base::State       *state;
                Motion            *parent;
                double             cost;
            };

            void freeMemory(void);

            double distanceFunction(const Motion* a, const Motion* b) const
            {
                return si_->distance(a->state, b->state);
            }

            base::ManifoldStateSamplerPtr                  sampler_;
            boost::shared_ptr< NearestNeighbors<Motion*> > nn_;

            double                                         goalBias_;
            double                                         maxDistance_;
            double                                         ballRadiusConst_;
            double                                         ballRadiusMax_;
            RNG                                            rng_;
        };

    }
}

#endif
