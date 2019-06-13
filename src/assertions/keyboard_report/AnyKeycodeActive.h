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

#include "assertions/ReportAssertion_.h"

namespace kaleidoscope {
namespace simulator {
namespace assertions {

/// @brief Asserts that any keycodes are active in the current report.
///
class AnyKeycodeActive {
   
   public:
      
      KT_ASSERTION_STD_CONSTRUCTOR(AnyKeycodeActive)
      
   private:
      
      class Assertion : public ReportAssertion_<KeyboardReport> {
            
         public:

            virtual void describe(const char *add_indent = "") const override {
               simulator_->log() << add_indent << "Any keycodes active";
            }

            virtual void describeState(const char *add_indent = "") const {
               simulator_->log() << add_indent << "Any keycodes active: ";
               simulator_->log() << this->getReport().isAnyKeyActive();
            }

            virtual bool evalInternal() override {
               return this->getReport().isAnyKeyActive();
            }
      };
   
   KT_AUTO_DEFINE_ASSERTION_INVENTORY(AnyKeycodeActive)
};

} // namespace assertions
} // namespace simulator
} // namespace kaleidoscope
