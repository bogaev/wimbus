#pragma once

#include "json.h"
#include <string>

namespace json {
using namespace std::literals;

enum class EntriesTypes {
	AFTER_CONSTRUCT = 0,
	START_DICT,
	KEY,
	VALUE,
	END_DICT,
	START_ARRAY,
	END_ARRAY,
};

class Builder {

	class DictItemContext;
	class KeyItemContext;
	class ArrayItemContext;
	class ContextBase {
	public:
		ContextBase(Builder& builder);
		DictItemContext StartDict();
		KeyItemContext Key(std::string key);
		Builder& Value(Node::Value value);
		Builder& EndDict();
		ArrayItemContext StartArray();
		Builder& EndArray();

	private:
		Builder& builder_;
	};
	class ArrayItemContext : public ContextBase {
	public:
		ArrayItemContext(Builder& builder) : ContextBase(builder) {};
		ArrayItemContext Value(Node::Value value);
		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
	};
	class DictItemContext : public ContextBase {
	public:
		DictItemContext(Builder& builder) : ContextBase(builder) {};
		DictItemContext StartDict() = delete;
		Builder& Value(Node::Value value) = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
	};
	class KeyItemContext : public ContextBase {
	public:
		KeyItemContext(Builder& builder) : ContextBase(builder) {};
		DictItemContext Value(Node::Value value);
		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
	};

public:
	DictItemContext StartDict();
	KeyItemContext Key(std::string key);
	Builder& Value(Node::Value value);
	Builder& EndDict();
	ArrayItemContext StartArray();
	Builder& EndArray();
	Node Build();

private:
	template<typename It>
	Node BuildDict(It& it);

	template<typename It>
	Node BuildArray(It& it);

	Node root_;
	std::vector<Node> nodes_stack_{};
	int dict_count = 0;
	int array_count = 0;
	EntriesTypes last_entry = EntriesTypes::AFTER_CONSTRUCT;
};


template<typename It>
Node Builder::BuildDict(It& it) {
	Dict dict;
	std::string key;
	int i = 0;
	for (it = std::next(it); *it != "}"s; ++it, ++i) {
		if (it->IsString() && it->AsString() == "{"s) {
			dict[key] = BuildDict(it);
			continue;
		}
		if (it->IsString() && it->AsString() == "["s) {
			dict[key] = BuildArray(it);
			continue;
		}
		i % 2 ? dict[key] = Node(*it) : key = (*it).AsString();
	}
	return Node(dict);
}

template<typename It>
Node Builder::BuildArray(It& it) {
	Array arr;
	for (it = std::next(it); *it != "]"s; ++it) {
		if (it->IsString() && it->AsString() == "{"s) {
			arr.push_back(BuildDict(it));
			continue;
		}
		if (it->IsString() && it->AsString() == "["s) {
			arr.push_back(BuildArray(it));
			continue;
		}
		arr.emplace_back(*it);
	}
	return Node(arr);
}

} //namespace json