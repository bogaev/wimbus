#include "json_builder.h"

namespace json {

// ContextBase class -----------------------------------------------------
Builder::ContextBase::ContextBase(Builder& builder) : builder_(builder) {}
Builder::DictItemContext Builder::ContextBase::StartDict() {
	return builder_.StartDict();
}
Builder::KeyItemContext Builder::ContextBase::Key(std::string key) {
	return builder_.Key(key);
}
Builder& Builder::ContextBase::Value(Node::Value value) {
	return builder_.Value(value);
}
Builder& Builder::ContextBase::EndDict() {
	return builder_.EndDict();
}
Builder::ArrayItemContext Builder::ContextBase::StartArray() {
	return builder_.StartArray();
}
Builder& Builder::ContextBase::EndArray() {
	return builder_.EndArray();
}

// KeyItemContext class -----------------------------------------------------
Builder::DictItemContext Builder::KeyItemContext::Value(Node::Value value) {
	return { ContextBase::Value(value) };
}

// ArrayItemContext class -----------------------------------------------------
Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
	return { ContextBase::Value(value) };
}

// Builder class -----------------------------------------------------

Builder::DictItemContext Builder::StartDict() {
	if ((last_entry != EntriesTypes::AFTER_CONSTRUCT) &&
		(last_entry != EntriesTypes::KEY) &&
		(last_entry != EntriesTypes::START_ARRAY) &&
		(last_entry != EntriesTypes::END_DICT) &&
		!(array_count != 0 && last_entry == EntriesTypes::VALUE)) {
		throw std::logic_error("StartDict error");
	}

	nodes_stack_.emplace_back("{"s);
	++dict_count;
	last_entry = EntriesTypes::START_DICT;
	return DictItemContext{ *this };
}

Builder::KeyItemContext Builder::Key(std::string key) {
	if ((last_entry == EntriesTypes::AFTER_CONSTRUCT) ||
		(last_entry == EntriesTypes::START_ARRAY) ||
		(dict_count == 0) ||
		(last_entry == EntriesTypes::KEY)) {
			throw std::logic_error("");
	}

	nodes_stack_.emplace_back(key);
	last_entry = EntriesTypes::KEY;
	return KeyItemContext{ *this };
}

Builder& Builder::Value(Node::Value value) {
	if ((last_entry != EntriesTypes::AFTER_CONSTRUCT) &&
		(last_entry != EntriesTypes::KEY) &&
		(last_entry != EntriesTypes::START_ARRAY) &&
		(last_entry != EntriesTypes::END_ARRAY) &&
		(last_entry != EntriesTypes::END_DICT) &&
		!(array_count != 0 && last_entry == EntriesTypes::VALUE)) {
			throw std::logic_error("Value error");
	}

	std::visit(
		[&](auto v) {
			nodes_stack_.emplace_back(v);
		},
		value);
	last_entry = EntriesTypes::VALUE;
	return *this;
}

Builder& Builder::EndDict() {
	if ((last_entry == EntriesTypes::AFTER_CONSTRUCT) || !dict_count) {
			throw std::logic_error("EndDict error");
	}

	nodes_stack_.emplace_back("}"s);
	--dict_count;
	last_entry = EntriesTypes::END_DICT;
	return *this;
}

Builder::ArrayItemContext Builder::StartArray() {
	if ((last_entry != EntriesTypes::AFTER_CONSTRUCT) &&
		(last_entry != EntriesTypes::START_ARRAY) &&
		(last_entry != EntriesTypes::KEY) &&
		(last_entry != EntriesTypes::END_DICT) &&
		(last_entry == EntriesTypes::END_ARRAY) &&
		!(array_count != 0 && last_entry == EntriesTypes::VALUE)) {
			throw std::logic_error("StartArray error");
	}

	nodes_stack_.emplace_back("["s);
	++array_count;
	last_entry = EntriesTypes::START_ARRAY;
	return ArrayItemContext{ *this };
}

Builder& Builder::EndArray() {
	if ((last_entry == EntriesTypes::AFTER_CONSTRUCT) ||
		(last_entry == EntriesTypes::START_DICT) ||
		(array_count == 0)) {
			throw std::logic_error("EndArray error");
	}

	nodes_stack_.emplace_back("]"s);
	--array_count;
	last_entry = EntriesTypes::END_ARRAY;
	return *this;
}

Node Builder::Build() {
	//билд вызван сразу после конструктора или не закрыты скобки контейнеров
	if (dict_count || array_count ||
		(last_entry == EntriesTypes::AFTER_CONSTRUCT)) {
			throw std::logic_error("Build error");
	}

	for (auto it = nodes_stack_.begin(); it != nodes_stack_.end(); ++it) {
		if (it->IsString() && it->AsString() == "{"s) {
			root_ = BuildDict(it);
			continue;
		}
		if (it->IsString() && it->AsString() == "["s) {
			root_ = BuildArray(it);
			continue;
		}
		root_ = Node(*it);
	}
	nodes_stack_.clear();
	return root_;
}

} //namespace json