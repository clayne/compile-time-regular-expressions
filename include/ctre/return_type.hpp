#ifndef CTRE__RETURN_TYPE__HPP
#define CTRE__RETURN_TYPE__HPP

#include "id.hpp"
#include <type_traits>
#include <tuple>
#include <string_view>

namespace ctre {
	
struct not_matched_tag_t { };

static constexpr inline auto not_matched = not_matched_tag_t{};
	
template <size_t Id, typename Name = void> struct captured_content {
	template <typename Iterator> struct storage {
		Iterator _begin{};
		Iterator _end{};
		bool _matched{false};
	
		using name = Name;
	
		constexpr storage() noexcept {}
	
		constexpr void matched() noexcept {
			_matched = true;
		}
		constexpr void set_start(Iterator pos) noexcept {
			_begin = pos;
		}
		constexpr void set_end(Iterator pos) noexcept {
			_end = pos;
		}
		constexpr Iterator get_end() const noexcept {
			return _end;
		}
		
	
		constexpr auto begin() const noexcept {
			return _begin;
		}
		constexpr auto end() const noexcept {
			return _end;
		}
	
		constexpr explicit operator bool() const noexcept {
			return _matched;
		}
		constexpr explicit operator auto() const noexcept {
			return to_view();
		}
		constexpr auto to_view() const noexcept {
			return std::basic_string_view{_begin, static_cast<size_t>(std::distance(_begin, _end))};
		}
		constexpr static size_t get_id() noexcept {
			return Id;
		}
	};
};

struct capture_not_exists_tag { };

static constexpr inline auto capture_not_exists = capture_not_exists_tag{};

template <typename... Captures> struct captures;

template <typename Head, typename... Tail> struct captures<Head, Tail...>: captures<Tail...> {
	Head head;
	template <size_t id> static constexpr bool exists() noexcept {
		if constexpr (id == Head::get_id()) {
			return true;
		} else {
			return captures<Tail...>::template exists<id>();
		}
	}
	template <size_t id> constexpr auto & select() noexcept {
		if constexpr (id == Head::get_id()) {
			return head;
		} else {
			return captures<Tail...>::template select<id>();
		}
	}
	template <typename Name> constexpr auto & select() noexcept {
		if constexpr (std::is_same_v<Name, Head::name>) {
			return head;
		} else {
			return captures<Tail...>::template select<Name>();
		}
	}
	template <size_t id> constexpr auto & select() const noexcept {
		if constexpr (id == Head::get_id()) {
			return head;
		} else {
			return captures<Tail...>::template select<id>();
		}
	}
	template <typename Name> constexpr auto & select() const noexcept {
		if constexpr (std::is_same_v<Name, Head::name>) {
			return head;
		} else {
			return captures<Tail...>::template select<Name>();
		}
	}
};

template <> struct captures<> {
	template <size_t> static constexpr bool exists() noexcept {
		return false;
	}
	template <size_t> constexpr auto & select() const noexcept {
		return capture_not_exists;
	}
	template <typename> constexpr auto & select() const noexcept {
		return capture_not_exists;
	}
};

template <typename Iterator, typename... Captures> struct regex_results {
	captures<captured_content<0>::template storage<Iterator>, typename Captures::template storage<Iterator>...> captures;
	
	constexpr regex_results() noexcept { }
	constexpr regex_results(not_matched_tag_t) noexcept { }
	
	template <size_t Id, typename = std::enable_if_t<decltype(captures)::template exists<Id>>> auto get() const noexcept {
		return captures.template select<Id>();
	}
	static constexpr size_t size() noexcept {
		return sizeof...(Captures) + 1;
	}
	constexpr regex_results & matched() noexcept {
		captures.template select<0>().matched();
		return *this;
	}
	constexpr explicit operator bool() const noexcept {
		return bool(captures.template select<0>());
	}
	constexpr explicit operator auto() const noexcept {
		return captures.template select<0>().to_view();
	}
	constexpr regex_results & set_start_mark(Iterator pos) noexcept {
		captures.template select<0>().set_start(pos);
		return *this;
	}
	constexpr regex_results & set_end_mark(Iterator pos) noexcept {
		captures.template select<0>().set_end(pos);
		return *this;
	}
	constexpr Iterator get_end_position() const noexcept {
		return captures.template select<0>().get_end();
	}
	template <size_t Id> constexpr regex_results & start_capture() noexcept {
		return *this;
	}
	template <size_t Id> constexpr regex_results & end_capture() noexcept {
		return *this;
	}
};

}

// support for structured bindings

namespace std {
	template <typename... Captures> struct tuple_size<ctre::regex_results<Captures...>> : std::integral_constant<size_t, ctre::regex_results<Captures...>::size()> { };
	
	template <size_t N, typename... Captures> struct tuple_element<N, ctre::regex_results<Captures...>> {
		using type = decltype(
			std::declval<const ctre::regex_results<Captures...> &>().template get<N>()
		);
	};
}

#endif