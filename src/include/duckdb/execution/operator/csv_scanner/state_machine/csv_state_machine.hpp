//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/csv_scanner/state_machine/csv_state_machine.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/operator/csv_scanner/util/csv_reader_options.hpp"
#include "duckdb/execution/operator/csv_scanner/buffer_manager/csv_buffer_manager.hpp"
#include "duckdb/execution/operator/csv_scanner/state_machine/csv_state_machine_cache.hpp"

namespace duckdb {

//! State of necessary CSV States to parse file
//! Current, previous, and state before the previous
struct CSVStates {
	void Initialize(CSVState initial_state) {
		current_state = initial_state;
		previous_state = initial_state;
		pre_previous_state = initial_state;
	}
	inline bool NewValue() {
		return previous_state == CSVState::DELIMITER;
	}

	inline bool NewRow() {
		return (previous_state == CSVState::RECORD_SEPARATOR && current_state != CSVState::EMPTY_LINE) ||
		       (current_state != CSVState::RECORD_SEPARATOR && previous_state == CSVState::CARRIAGE_RETURN);
	}

	inline bool IsCurrentNew() {
		return current_state == CSVState::RECORD_SEPARATOR || current_state == CSVState::CARRIAGE_RETURN;
	}

	inline bool EmptyLine() {
		return current_state == CSVState::EMPTY_LINE;
	}

	inline bool IsInvalid() {
		return current_state == CSVState::INVALID;
	}
	CSVState current_state = CSVState::EMPTY_LINE;
	CSVState previous_state = CSVState::EMPTY_LINE;
	CSVState pre_previous_state = CSVState::EMPTY_LINE;
};

//! The CSV State Machine comprises a state transition array (STA).
//! The STA indicates the current state of parsing based on both the current and preceding characters.
//! This reveals whether we are dealing with a Field, a New Line, a Delimiter, and so forth.
//! The STA's creation depends on the provided quote, character, and delimiter options for that state machine.
//! The motivation behind implementing an STA is to remove branching in regular CSV Parsing by predicting and detecting
//! the states. Note: The State Machine is currently utilized solely in the CSV Sniffer.
class CSVStateMachine {
public:
	//! Mutex to lock when getting next batch of bytes (Parallel Only)
	mutex main_mutex;

	explicit CSVStateMachine(CSVReaderOptions &options_p, const CSVStateMachineOptions &state_machine_options,
	                         CSVStateMachineCache &csv_state_machine_cache_p);

	explicit CSVStateMachine(const StateMachine &transition_array, const CSVReaderOptions &options);

	//! Transition all states to next state, that depends on the current char
	inline void Transition(CSVStates &states, char current_char) const {
		states.pre_previous_state = states.previous_state;
		states.previous_state = states.current_state;
		states.current_state = transition_array[states.current_state][static_cast<uint8_t>(current_char)];
	}

	const vector<SelectionVector> &GetSelectionVector();
	//! The Transition Array is a Finite State Machine
	//! It holds the transitions of all states, on all 256 possible different characters
	const StateMachine &transition_array;
	//! Options of this state machine
	const CSVStateMachineOptions state_machine_options;
	//! CSV Reader Options
	const CSVReaderOptions &options;

	//! Stores identified start row for this file (e.g., a file can start with garbage like notes, before the header)
	idx_t start_row = 0;

	//! Both these variables are used for new line identifier detection
	bool single_record_separator = false;
	bool carry_on_separator = false;

	//! Dialect options resulting from sniffing
	DialectOptions dialect_options;

	vector<SelectionVector> selection_vector;
};

} // namespace duckdb
