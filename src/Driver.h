/* -*- mode: c++ -*-
 * Kaleidoscope-Testing -- A C++ testing API for the Kaleidoscope keyboard 
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

#include "AssertionQueue.h"
#include "KeyboardReport.h"

#include <vector>

namespace kaleidoscope {
namespace testing {
   
class Driver;
class _Assertion;

/// @brief An abstract driver output stream.
///
class DriverStream_ {
   
   public:
      
      struct Endl {};
      
      DriverStream_(const Driver *driver) : driver_(driver) {}
      
      virtual ~DriverStream_() {}

   protected:
      
      std::ostream &getOStream() const;
      
      void checkLineStart();
      
      virtual void reactOnLineStart();
      virtual void reactOnLineEnd();
      
      template<typename _T>
      void output(const _T &t) {
         this->checkLineStart();
         this->getOStream() << t;
      }
      
      void output(const Endl &) {
         line_start_ = true;
         this->reactOnLineEnd();
         this->getOStream() << std::endl;
      }
      
   protected:
      
      const Driver *driver_;
      
   private:
      
      bool line_start_ = true;
};
   
/// @brief A stream class for error output.
///
class ErrorStream : public DriverStream_ {
   
   public:
      
      ErrorStream(const Driver *driver);
      
      template<typename _T>
      ErrorStream &operator<<(const _T &t) { 
         this->output(t);
         return *this; 
      }
      
      virtual ~ErrorStream() override;

   private:
   
      virtual void reactOnLineStart() override;
};

/// @brief A stream class for log output.
///
class LogStream : public DriverStream_ {
   
   public:
      
      LogStream(const Driver *driver);
      virtual ~LogStream() override;
      
      template<typename _T>
      LogStream &operator<<(const _T &t) { 
         this->output(t);
         return *this; 
      }
};

/// @brief A stream class that generates formatted text headers in log output.
///
class HeaderStream : public DriverStream_ {
   
   public:
      
      HeaderStream(const Driver *driver);
      
      template<typename _T>
      HeaderStream &operator<<(const _T &t) { 
         this->output(t);
         return *this; 
      }
      
      virtual ~HeaderStream() override;
      
   private:
   
      virtual void reactOnLineStart() override;
};

/// @brief A class that represents an individual test.
///
class Test {
   
   public:
      
      Test(Driver *driver, const char *name);
      ~Test();
      
   private:
      
      Driver *driver_;
      const char *name_;
      int error_count_start_;
};
        
/// @ brief The main test driver object.
///
class Driver {
   
   public:
      
      typedef int TimeType;
   
   private:
      
      std::ostream *out_;
      bool debug_;
      int cycle_duration_;
      bool abort_on_first_error_;
      
      bool assertions_passed_ = true;
      int n_keyboard_reports_in_cycle_ = 0;
      int n_overall_keyboard_reports_ = 0;
   
      int cycle_id_ = 0;
      TimeType time_ = .0;
      int scan_cycles_default_count_ = 5;
      
      mutable int error_count_ = 0;
      
      bool error_if_report_without_queued_assertions_ = false;
      
      AssertionQueue queued_keyboard_report_assertions_;
      AssertionQueue permanent_keyboard_report_assertions_;
      AssertionQueue queued_cycle_assertions_;
      AssertionQueue permanent_cycle_assertions_;
      
      KeyboardReport current_keyboard_report_;
      
      class KeyboardReportConsumer : public KeyboardReportConsumer_
      {
         public:
            
            KeyboardReportConsumer(Driver &driver) : driver_(driver) {}

            virtual void processKeyboardReport(
                           const HID_KeyboardReport_Data_t &report_data) override;
                           
         private:
            
            Driver &driver_;
            
      } keyboard_report_consumer_;
      
      friend class KeyboardReportConsumer;
      
   public:
      
      /// @brief Constructor.
      ///
      /// @param out The output stream that is used for all output.
      /// @param debug Generates additional debug information if enabled.
      /// @param cycle_duration The duration in ms of one scan cycle.
      /// @param abort_on_first_error If enabled, testing is aborted after
      ///        the first error occurred.
      ///
      Driver(std::ostream &out, 
             bool debug, 
             int cycle_duration = 5, 
             bool abort_on_first_error = false);
      
      ~Driver();
      
      /// @details If the ErrorIfReportWithoutQueuedAssertions is enabled
      ///          it is considered an error if no assertions are queued
      ///          for a report.
      /// @param state The new boolean state of the condition.
      ///
      void setErrorIfReportWithoutQueuedAssertions(bool state) {
         error_if_report_without_queued_assertions_ = state;
      }
      
      /// @brief Retreives the state of the ErrorIfReportWithoutQueuedAssertions
      ///        condition.
      /// @returns The conditions state.
      ///
      bool getErrorIfReportWithoutQueuedAssertions() const {
         return error_if_report_without_queued_assertions_;
      }
      
      /// @brief Retreives the queued keyboard report assertions.
      /// @details The head of the assertion queue is applied to the
      ///        next keyboard report. It is removed from the queue afterwards.
      ///
      AssertionQueue &queuedKeyboardReportAssertions() {
         return queued_keyboard_report_assertions_;
      }
      
      /// @brief Retreives the permanent keyboard report assertions.
      /// @details Permanent keyboard report assertions are applied to 
      ///        every keyboard report.
      ///
      AssertionQueue &permanentKeyboardReportAssertions() {
         return permanent_keyboard_report_assertions_;
      }
      
      /// @brief Retreives the queued cycle assertions.
      /// @details The head of the assertion queue is applied at the end of
      ///        the next cycle and removed afterwards.
      ///
      AssertionQueue &queuedCycleAssertions() {
         return queued_cycle_assertions_;
      }
      
      /// @brief Retreives the permanent cycle assertions.
      /// @details Permanent cycle assertions are applied after every cycle.
      ///
      AssertionQueue &permanentCycleAssertions() {
         return permanent_cycle_assertions_;
      }
      
      /// @brief Registers a key press event.
      ///
      /// @param row The keyboard matrix row.
      /// @param col The keyboard matrix col.
      ///
      void pressKey(uint8_t row, uint8_t col);
       
      /// @brief Registers a key release event. 
      ///
      /// @details Make sure that the key was registered
      ///          as pressed, using pressKey(...), before calling this method.
      ///
      /// @param row The keyboard matrix row.
      /// @param col The keyboard matrix col.
      ///
      void releaseKey(uint8_t row, uint8_t col);
      
      /// brief Registers a key being tapped.
      ///
      /// @param row The keyboard matrix row.
      /// @param col The keyboard matrix col.
      ///
      void tapKey(uint8_t row, uint8_t col);
      
      /// @brief Taps a key a number of times.
      ///
      /// @details After each tap, a specified number of cycles 
      ///          are processed automatically before the next tap is issued.
      ///
      /// @param num_taps The number of taps.
      /// @param row The keyboard matrix row.
      /// @param col The keyboard matrix col.
      /// @param tap_interval_cycles The number of cycles that are made 
      ///        elapsing between individual taps.
      ///
      void multiTapKey(int num_taps, uint8_t row, uint8_t col, 
                       int tap_interval_cycles = 1,
                       std::shared_ptr<_Assertion> after_tap_and_cycles_assertion = std::shared_ptr<_Assertion>()
                      );

      /// @brief Releases all keys that are currently pressed.
      ///
      void clearAllKeys();
         
      /// @brief Runs a scan cycle and processes assertions afterwards.
      ///
      void cycle();
         
      /// @brief Runs a number of scan cycles and processes assertions afterwards.
      ///
      /// @param n The number of cycles to run.
      /// @param at_end_assertion_list A list of assertions to be evaluated 
      ///        at the end of cycling. They are discarded after evaluation.
      /// @param cycle_assertion_list A list of assertions that are evaluated
      ///        after every cycle.
      ///
      template<typename..._Assertions>
      void cycles(int n = 0, _Assertions...assertions) {
         this->cyclesInternal(n,
            std::vector<std::shared_ptr<_Assertion>>{
               std::forward<_Assertions>(assertions)...
            }
         );
      }
            
      /// @brief Skips a given amount of time by running cycles.
      ///
      /// @param delta_t A time interval in [ms] that is supposed to be skipped.
      ///
      void advanceTime(TimeType delta_t);
      
      /// @brief Runs keyboard scan cycles until a specified point in time.
      ///
      /// @param time The target time in [ms].
      ///
      void cycleTo(TimeType time);
      
      /// @brief Immediately evaluates a number of assertions
      ///
      /// @tparam assertions A number assertions to be evaluated immediately.
      ///
      template<typename..._Assertions>
      void evaluateAssertions(_Assertions...assertions) {
         this->evaluateAssertionsInternal(
            std::vector<std::shared_ptr<_Assertion>>{
               std::forward<_Assertions>(assertions)...
            }
         );
      }
            
      /// @brief Retreives a stream object for log output.
      ///
      /// @returns The log stream object.
      /// 
      LogStream log() const { return LogStream{this}; }
      
      /// @brief Retreives a log stream for header output.
      ///
      /// @returns The header log stream object.
      /// 
      HeaderStream header() const { return HeaderStream{this}; }
         
      /// @brief Retrieves a log stream for error output.
      ///
      /// @returns The error log stream object.
      ///
      ErrorStream error() const { 
         ++error_count_;
         return ErrorStream{this}; 
      }
      
      /// @brief Retreives the total number of errors that were registered.
      ///
      int getErrorCount() const { return error_count_; }
      
      /// @brief Resets the keyboard to initial state.
      ///
      void initKeyboard();
         
      /// @brief Retreives the current keyboard report.
      ///
      const KeyboardReport &getCurrentKeyboardReport() const {
         return current_keyboard_report_;
      }
      
      /// @brief Retreives the state of the AbortOnFirstError condition.
      ///
      bool getAbortOnFirstError() const { return abort_on_first_error_; }
      
      /// @brief Retreives the number of keyboard reports that were generated
      ///        in the current cycle.
      ///
      int getNumKeyboardReportsInCycle() const { return n_keyboard_reports_in_cycle_; }
      
      /// @brief Retreives the number of overall keyboard reports that were
      ///        generated since the start of testing.
      ///
      int getNumOverallKeyboardReports() const { return n_overall_keyboard_reports_; }
      
      /// @brief Retreives the current time [ms].
      ///
      TimeType getTime() const { return time_; }
      
      /// @brief Retreives the current cycle id.
      ///
      int getCycleId() const { return cycle_id_; }
      
      /// @brief Sets debugging output state. If enabled, additional (verbose)
      ///        output is generated.
      ///
      /// @param state The new state of debugging output.
      ///
      void setDebug(bool state) { debug_ = state; }
      
      /// @brief Retreives the current debugging state.
      ///
      bool getDebug() const { return debug_; }
      
      /// @brief Asserts that no assertions (keyboard report and cycle)
      ///        are currently queued.
      /// @details This function is automatically called at the end of each 
      ///        test to make sure that all assertions were used.
      ///
      void assertNothingQueued() const;
      
      /// @brief Generates a test.
      /// @details The lifetime of the returned test object decides about
      ///        the duration of a test.
      /// @returns The test object. Add additonal scope if necessary to 
      ///        restrict the lifetime of test objects.
      ///
      Test newTest(const char *name) {
         return Test{this, name};
      }
      
      /// @brief Asserts a boolean condition.
      /// @param cond The condition.
      /// @param assertion_code A string representation of the assertion.
      ///
      void assertCondition(bool cond, const char *condition_string) const;
      
      /// @brief Changes the duration in [ms] of every simulated cycle.
      /// @param dt The new duration [ms].
      ///
      void setCycleDuration(TimeType dt) {
         cycle_duration_ = dt;
      }
      
      /// @brief Retreives the currently defined cycle duration.
      /// @returns The cycle duration in [ms].
      ///
      TimeType getCycleDuration() const {
         return cycle_duration_;
      }
      
      /// @brief Resets the drivers output stream.
      /// @details This might serve to redirect output to a file
      ///        by using a std::ofstream.
      /// @param out The new ostream object.
      ///
      void setOStream(std::ostream &out) {
         out_ = &out;
      }
      
      /// @brief Retreives the currently associated ostream object.
      ///
      std::ostream &getOStream() const { return *out_; }  
      
   private:
      
      bool checkStatus() const;
            
      void headerText();
      
      void footerText();
      
      void processKeyboardReport(const HID_KeyboardReport_Data_t &report_data);
            
      void processKeyboardReportAssertion(const std::shared_ptr<_Assertion> &assertion);
                 
      void cycleInternal(bool only_log_reports = false);
      
      void checkCycleDurationSet();
      
      // This method is templated to enable it being used for std::vector
      // and std::deque.
      //
      template<typename _Container>
      void evaluateAssertionsInternal(const _Container &assertions) {
            
         if(assertions.empty()) { return; }
         
         for(auto &assertion: assertions) {
      
            // Just in case we haven't done that before
            //
            assertion->setDriver(this);
            
            bool assertion_passed = assertion->eval();
            
            if(!assertion_passed || debug_) {
               assertion->report();
            }
            
            assertions_passed_ &= assertion_passed;
         }
      }
      
      std::string generateCycleInfo() const;
      
      void skipTimeInternal(TimeType delta_t);
      
      void cyclesInternal(int n, 
                  const std::vector<std::shared_ptr<_Assertion>> &cycle_assertion_list);
};

/// @brief Asserts a condition.
/// @details Use this macro instead of the Driver class' assertCondition(...)
///        method to enable string output of the condition code in
///        error messages.
///
#define KT_ASSERT_CONDITION(DRIVER, ...)                                       \
   DRIVER.assertCondition((__VA_ARGS__), #__VA_ARGS__)
      
} // namespace testing
} // namespace kaleidoscope