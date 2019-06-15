/* -*- mode: c++ -*-
 * Kaleidoscope-Simulator -- A C++ testing API for the Kaleidoscope keyboard 
 *                         firmware.
 * Copyright (C) 2019  noseglasses (shinynoseglasses@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "assertions/generic_report/ReportAssertion.h"

namespace kaleidoscope {
namespace simulator {
namespace assertions {

/// @brief Asserts that the current report is the nth report in the current cycle.
///
class ReportNthInCycle {
   
   public:
      
      /// @brief Constructor.
      /// @param report_id The id of the report to check against.
      ///
      ReportNthInCycle(int report_id)
         : ReportNthInCycle(DelegateConstruction{}, report_id)
      {}
      
   private:
      
      class Assertion : public ReportAssertion_ {
            
         public:

            Assertion(int report_id) : report_id_(report_id) {}

            virtual void describe(const char *add_indent = "") const override {
               this->getSimulator()->log() << add_indent << "Report " << report_id_ << ". in cycle";
            }

            virtual void describeState(const char *add_indent = "") const {
               this->getSimulator()->log() << add_indent << "Report is " 
                  << this->getSimulator()->getNumReportsInCycle() << ". in cycle";
            }

            virtual bool evalInternal() override {
               return this->getSimulator()->getNumReportsInCycle() == report_id_;
            }
            
         private:
            
            int report_id_ = -1;
      };
   
   KT_AUTO_DEFINE_ASSERTION_INVENTORY(ReportNthInCycle)
};

} // namespace assertions
} // namespace simulator
} // namespace kaleidoscope