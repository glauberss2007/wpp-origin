/*
 * Copyright 2015-2017 Kevin Wojniak
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "mustache.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace kainjow::mustache;

#ifndef MUSTACHE_VS2013
    #if defined(_MSC_VER) && _MSC_VER == 1800
        #define MUSTACHE_VS2013 1
    #else
        #define MUSTACHE_VS2013 0
    #endif
#endif

TEST_CASE("split") {

    std::vector<std::string> names;
    names = split<std::string>("", '.');
    REQUIRE(names.size() == 0);
    names = split<std::string>("test", '.');
    REQUIRE(names.size() == 1);
    CHECK(names[0] == "test");
    names = split<std::string>("a.b", '.');
    REQUIRE(names.size() == 2);
    CHECK(names[0] == "a");
    CHECK(names[1] == "b");
    names = split<std::string>(".", '.');
    REQUIRE(names.size() == 1);
    CHECK(names[0] == "");
    names = split<std::string>("a.", '.');
    REQUIRE(names.size() == 1);
    CHECK(names[0] == "a");

}

TEST_CASE("variables") {

    SECTION("empty") {
        mustache tmpl("");
        data data;
        CHECK(tmpl.render(data).empty());
    }

    SECTION("none") {
        mustache tmpl("Hello");
        data data;
        CHECK(tmpl.render(data) == "Hello");
    }

    SECTION("single_miss") {
        mustache tmpl("Hello {{name}}");
        data data;
        CHECK(tmpl.render(data) == "Hello ");
    }

    SECTION("single_exist") {
        mustache tmpl("Hello {{name}}");
        data data;
        data.set("name", "Steve");
        CHECK(tmpl.render(data) == "Hello Steve");
    }

    SECTION("single_exist_wide") {
        mustachew tmpl(L"Hello {{name}}");
        dataw data;
        data.set(L"name", L"Steve");
        CHECK(tmpl.render(data) == L"Hello Steve");
    }

    SECTION("escape") {
        mustache tmpl("Hello {{name}}");
        data data;
        data.set("name", "\"S\"<br>te&v\'e");
        CHECK(tmpl.render(data) == "Hello &quot;S&quot;&lt;br&gt;te&amp;v&apos;e");
    }
    
    SECTION("unescaped1") {
        mustache tmpl("Hello {{{name}}}");
        data data;
        data.set("name", "\"S\"<br>te&v\'e");
        CHECK(tmpl.render(data) == "Hello \"S\"<br>te&v\'e");
    }

    SECTION("unescaped2") {
        mustache tmpl("Hello {{&name}}");
        data data;
        data.set("name", "\"S\"<br>te&v\'e");
        CHECK(tmpl.render(data) == "Hello \"S\"<br>te&v\'e");
    }

    SECTION("unescaped2_spaces") {
        mustache tmpl("Hello {{   &      name  }}");
        data data;
        data.set("name", "\"S\"<br>te&v\'e");
        CHECK(tmpl.render(data) == "Hello \"S\"<br>te&v\'e");
    }

    SECTION("empty_name") {
        mustache tmpl("Hello {{}}");
        data data;
        data.set("", "Steve");
        CHECK(tmpl.render(data) == "Hello Steve");
    }

    SECTION("braces") {
        mustache tmpl("my {{var}}");
        data data;
        data.set("var", "{{te}}st");
        CHECK(tmpl.render(data) == "my {{te}}st");
    }

}

TEST_CASE("comments") {
    
    SECTION("simple") {
        mustache tmpl("<h1>Today{{! ignore me }}.</h1>");
        data data;
        CHECK(tmpl.render(data) == "<h1>Today.</h1>");
    }

    SECTION("newlines") {
        mustache tmpl("Hello\n{{! ignore me }}\nWorld\n");
        data data;
        CHECK(tmpl.render(data) == "Hello\n\nWorld\n");
    }

}

TEST_CASE("set_delimiter") {

    SECTION("basic") {
        mustache tmpl("{{name}}{{=<% %>=}}<% name %><%={{ }}=%>{{ name }}");
        data data;
        data.set("name", "Steve");
        CHECK(tmpl.render(data) == "SteveSteveSteve");
    }

    SECTION("small") {
        mustache tmpl("{{n}}{{=a b=}}anba={{ }}=b{{n}}");
        data data;
        data.set("n", "s");
        CHECK(tmpl.render(data) == "sss");
    }
    
    SECTION("noreset") {
        mustache tmpl("{{=[ ]=}}[name] [x] + [y] = [sum]");
        data data;
        data.set("name", "Steve");
        data.set("x", "1");
        data.set("y", "2");
        data.set("sum", "3");
        CHECK(tmpl.render(data) == "Steve 1 + 2 = 3");
    }
    
    SECTION("whitespace") {
        mustache tmpl("|{{= @   @ =}}|");
        data data;
        REQUIRE(tmpl.is_valid());
        CHECK(tmpl.render(data) == "||");
    }

}

TEST_CASE("sections") {
    
    SECTION("nonexistant") {
        mustache tmpl("{{#var}}not shown{{/var}}");
        data data;
        CHECK(tmpl.render(data) == "");
    }

    SECTION("false") {
        mustache tmpl("{{#var}}not shown{{/var}}");
        data dat;
        dat.set("var", data(data::type::bool_false));
        CHECK(tmpl.render(dat) == "");
    }

    SECTION("emptylist") {
        mustache tmpl("{{#var}}not shown{{/var}}");
        data dat;
        dat.set("var", data(data::type::list));
        CHECK(tmpl.render(dat) == "");
    }
    
    SECTION("nested") {
        mustache tmpl("{{#var1}}hello{{#var2}}world{{/var2}}{{/var1}}");
        data data;
        data.set("var1", data::type::bool_true);
        data.set("var2", data::type::bool_true);
        CHECK(tmpl.render(data) == "helloworld");
    }

}

TEST_CASE("sections_inverted") {
    
    SECTION("nonexistant") {
        mustache tmpl("{{^var}}shown{{/var}}");
        CHECK(tmpl.render(data()) == "shown");
    }
    
    SECTION("false") {
        mustache tmpl("{{^var}}shown{{/var}}");
        data dat("var", data(data::type::bool_false));
        CHECK(tmpl.render(dat) == "shown");
    }
    
    SECTION("emptylist") {
        mustache tmpl("{{^var}}shown{{/var}}");
        data dat("var", data(data::type::list));
        CHECK(tmpl.render(dat) == "shown");
    }
    
}

TEST_CASE("section_lists") {
    
    SECTION("list") {
        mustache tmpl("{{#people}}Hello {{name}}, {{/people}}");
        data people = data::type::list;
        for (auto& name : {"Steve", "Bill", "Tim"}) {
            people.push_back(data("name", name));
        }
        data data("people", people);
        CHECK(tmpl.render(data) == "Hello Steve, Hello Bill, Hello Tim, ");
    }
    
    SECTION("nested") {
        mustache tmpl("{{#families}}surname={{surname}}, members={{#members}}{{given}},{{/members}}|{{/families}}");
        data families = data::type::list;
        data family1;
        family1.set("surname", "Smith");
        data members1 = data::type::list;
        data member1a; member1a.set("given", "Steve"); members1.push_back(member1a);
        data member1b; member1b.set("given", "Joe"); members1.push_back(member1b);
        family1.set("members", members1);
        data family2;
        family2.set("surname", "Lee");
        data members2 = data::type::list;
        data member2a; member2a.set("given", "Bill"); members2.push_back(member2a);
        data member2b; member2b.set("given", "Peter"); members2.push_back(member2b);
        family2.set("members", members2);
        families.push_back(family1);
        families.push_back(family2);
        data data;
        data.set("families", families);
        CHECK(tmpl.render(data) == "surname=Smith, members=Steve,Joe,|surname=Lee, members=Bill,Peter,|");
    }
    
    SECTION("dot") {
        mustache tmpl("{{#names}}Hello {{.}}, {{/names}}");
        data names = data::type::list;
        names.push_back(data("Steve"));
        names.push_back(data("Bill"));
        names.push_back(data("Tim"));
        data data("names", names);
        CHECK(tmpl.render(data) == "Hello Steve, Hello Bill, Hello Tim, ");
    }

    SECTION("dot2") {
        mustache tmpl("{{#names}}Hello {{.}}{{/names}}{{#friends}} and {{.}}{{/friends}}");
        data friends = data::type::list;
        friends.push_back("Bill");
        friends.push_back("Tim");
        data data;
        data.set("friends", friends);
        CHECK(tmpl.render(data) == " and Bill and Tim");
        data.set("names", "Steve");
        CHECK(tmpl.render(data) == "Hello Steve and Bill and Tim");
    }

}

TEST_CASE("section_object") {
    
    SECTION("basic") {
        mustache tmpl("{{#employee}}name={{name}}, age={{age}}{{/employee}}");
        data person;
        person.set("name", "Steve");
        person.set("age", "42");
        data data;
        data.set("employee", person);
        CHECK(tmpl.render(data) == "name=Steve, age=42");
    }

    SECTION("basic_parent") {
        mustache tmpl("({{subject}}) {{#employee}}name={{name}}, age={{age}} - {{subject}}{{/employee}}");
        data person;
        person.set("name", "Steve");
        person.set("age", "42");
        person.set("subject", "email");
        data data;
        data.set("subject", "test");
        data.set("employee", person);
        CHECK(tmpl.render(data) == "(test) name=Steve, age=42 - email");
    }

}

TEST_CASE("examples") {
    
    SECTION("one") {
        mustache tmpl{"Hello {{what}}!"};
        std::cout << tmpl.render({"what", "World"}) << std::endl;
        CHECK(tmpl.is_valid());
        CHECK(tmpl.error_message() == "");
        CHECK(tmpl.render({"what", "World"}) == "Hello World!");
    }

    SECTION("two") {
        mustache tmpl{"{{#employees}}{{name}}, {{/employees}}"};
        data employees{data::type::list};
        employees << data{"name", "Steve"} << data{"name", "Bill"};
        std::ostream& stream = tmpl.render({"employees", employees}, std::cout) << std::endl;
        CHECK(tmpl.is_valid());
        CHECK(tmpl.error_message() == "");
        CHECK(tmpl.render({"employees", employees}) == "Steve, Bill, ");
        CHECK(&stream == &std::cout);
    }

    SECTION("three") {
        mustache tmpl("Hello {{what}}!");
        std::stringstream ss;
        tmpl.render({"what", "World"}, [&ss](const std::string& str) {
            ss << str;
        });
        CHECK(tmpl.is_valid());
        CHECK(tmpl.error_message() == "");
        CHECK(ss.str() == "Hello World!");
    }

}

TEST_CASE("data") {

    SECTION("types") {
        data dat("age", "42");
        data emptyStr = data::type::string;
        dat["name"] = "Steve";
        dat["is_human"] = data::type::bool_true;
        dat["is_dog"] = false;
        dat["is_organic"] = true;
        const data* name;
        const data* age;
        const data* is_human;
        name = dat.get("name");
        age = dat.get("age");
        is_human = dat.get("is_human");
        CHECK(name != (const data*)0);
        CHECK(age != (const data*)0);
        CHECK(is_human != (const data*)0);
        CHECK(dat.get("miss") == (const data*)0);
        REQUIRE(name->is_string());
        CHECK(name->string_value() == "Steve");
        REQUIRE(age->is_string());
        CHECK(age->string_value() == "42");
        CHECK(is_human->is_true());
        CHECK(is_human->is_bool());
        CHECK(emptyStr.is_string());
        CHECK(emptyStr.string_value() == "");
        CHECK(dat["is_dog"].is_bool());
        CHECK(dat["is_dog"].is_false());
        CHECK(dat["is_organic"].is_bool());
        CHECK(dat["is_organic"].is_true());
    }

    SECTION("move_ctor") {
        data obj1{data::type::list};
        CHECK(obj1.is_list());
        data obj2{std::move(obj1)};
        CHECK(obj2.is_list());
        CHECK(obj1.is_invalid());
        obj2.push_back({"name", "Steve"}); // this should puke if the internal data isn't setup correctly
    }

    SECTION("move_assign") {
        data obj1{data::type::list};
        CHECK(obj1.is_list());
        data obj2 = std::move(obj1);
        CHECK(obj2.is_list());
        CHECK(obj1.is_invalid());
        obj2.push_back({"name", "Steve"}); // this should puke if the internal data isn't setup correctly

        data lambda0{lambda{[](const std::string&){ return ""; }}};
        CHECK(lambda0.is_lambda());
        data lambda1 = std::move(lambda0);
        CHECK(lambda1.is_lambda());
        CHECK(lambda0.is_invalid());

        data lambda3{lambda2{[](const std::string&, const renderer&){ return ""; }}};
        CHECK(lambda3.is_lambda2());
        data lambda4 = std::move(lambda3);
        CHECK(lambda4.is_lambda2());
        CHECK(lambda3.is_invalid());
    }

    SECTION("lambda_copy_ctor") {
        data l1{lambda{[](const std::string&){ return ""; }}};
        data l2{l1};
        CHECK(l1.is_lambda());
        CHECK(l2.is_lambda());
        CHECK(l1.lambda_value()("") == l2.lambda_value()(""));
    }

    SECTION("lambda2_copy_ctor") {
        data l1{lambda2{[](const std::string&, const renderer& r){ return r(""); }}};
        data l2{l1};
        CHECK(l1.is_lambda2());
        CHECK(l2.is_lambda2());
    }

}

TEST_CASE("errors") {

    SECTION("unclosed_section") {
        mustache tmpl("test {{#employees}}");
        CHECK_FALSE(tmpl.is_valid());
        CHECK(tmpl.error_message() == "Unclosed section \"employees\" at 5");
    }
    
    SECTION("unclosed_section_nested") {
        mustache tmpl("{{#var1}}hello{{#var2}}world");
        data data;
        data.set("var1", data::type::bool_true);
        data.set("var2", data::type::bool_true);
        CHECK(tmpl.render(data) == "");
        CHECK(tmpl.is_valid() == false);
        CHECK(tmpl.error_message() == "Unclosed section \"var1\" at 0");
    }

    SECTION("unclosed_section_nested2") {
        mustache tmpl("{{#var1}}hello{{#var2}}world{{/var1}}");
        data data;
        data.set("var1", data::type::bool_true);
        data.set("var2", data::type::bool_true);
        CHECK(tmpl.render(data) == "");
        CHECK(tmpl.is_valid() == false);
        CHECK(tmpl.error_message() == "Unclosed section \"var1\" at 0");
    }

    SECTION("unclosed_tag") {
        mustache tmpl("test {{employees");
        CHECK_FALSE(tmpl.is_valid());
        CHECK(tmpl.error_message() == "Unclosed tag at 5");
    }
    
    SECTION("unopened_section") {
        mustache tmpl("test {{/employees}}");
        CHECK_FALSE(tmpl.is_valid());
        CHECK(tmpl.error_message() == "Unopened section \"employees\" at 5");
    }
    
    SECTION("invalid_set_delimiter") {
        std::vector<std::string> invalids;
        invalids.push_back("test {{=< =}}");  // not 5 characters
        invalids.push_back("test {{=....}}"); // not ending with =
        invalids.push_back("test {{=...=}}"); // does not contain space
        invalids.push_back("test {{=.  ==}}"); // can't contain equal sign
        invalids.push_back("test {{==  .=}}"); // can't contain equal sign
        invalids.push_back("test {{=[ ] ] ]=}}"); // can't contain space
        invalids.push_back("test {{=[ [ ]=}}"); // can't contain space
        std::vector<std::string>::size_type total = 0;
        for (const auto& str: invalids) {
            mustache tmpl(str);
            CHECK_FALSE(tmpl.is_valid());
            CHECK(tmpl.error_message() == "Invalid set delimiter tag at 5");
            ++total;
        }
        CHECK(total == invalids.size());
        CHECK(total == 7);
    }
    
    SECTION("lambda") {
        mustache tmpl{"Hello {{lambda}}!"};
        data dat("lambda", data{lambda{[](const std::string&){
            return "{{#what}}";
        }}});
        CHECK(tmpl.is_valid() == true);
        CHECK(tmpl.error_message() == "");
        CHECK(tmpl.render(dat) == "Hello ");
        CHECK(tmpl.is_valid() == false);
        CHECK(tmpl.error_message() == "Unclosed section \"what\" at 0");
    }

    SECTION("lambda2") {
        mustache tmpl{"Hello {{lambda}}!"};
        data dat("lambda", data{lambda{[](const std::string&){
            return "{{what}}";
        }}});
        dat["what"] = data{lambda{[](const std::string&){
            return "{{#blah}}";
        }}};
        CHECK(tmpl.is_valid() == true);
        CHECK(tmpl.error_message() == "");
        CHECK(tmpl.render(dat) == "Hello ");
        CHECK(tmpl.is_valid() == false);
        CHECK(tmpl.error_message() == "Unclosed section \"blah\" at 0");
    }

    SECTION("partial") {
        mustache tmpl{"Hello {{>partial}}!"};
        data dat("partial", data{partial{[](){
            return "{{#what}}";
        }}});
        CHECK(tmpl.is_valid() == true);
        CHECK(tmpl.error_message() == "");
        CHECK(tmpl.render(dat) == "Hello ");
        CHECK(tmpl.is_valid() == false);
        CHECK(tmpl.error_message() == "Unclosed section \"what\" at 0");
    }

    SECTION("partial2") {
        mustache tmpl{"Hello {{>partial}}!"};
        data data("partial", {partial{[](){
            return "{{what}}";
        }}});
        data["what"] = lambda{[](const std::string&){
            return "{{#blah}}";
        }};
        CHECK(tmpl.is_valid() == true);
        CHECK(tmpl.error_message() == "");
        CHECK(tmpl.render(data) == "Hello ");
        CHECK(tmpl.is_valid() == false);
        CHECK(tmpl.error_message() == "Unclosed section \"blah\" at 0");
    }
    
    SECTION("section_lambda") {
        mustache tmpl{"{{#what}}asdf{{/what}}"};
        data data("what", lambda{[](const std::string&){
            return "{{blah";
        }});
        CHECK(tmpl.is_valid() == true);
        CHECK(tmpl.error_message() == "");
        CHECK(tmpl.render(data) == "");
        CHECK(tmpl.is_valid() == false);
        CHECK(tmpl.error_message() == "Unclosed tag at 0");
    }

}

TEST_CASE("partials") {

    SECTION("empty") {
        mustache tmpl{"{{>header}}"};
        data data;
        CHECK(tmpl.render(data) == "");
    }

    SECTION("basic") {
        mustache tmpl{"{{>header}}"};
        partial part = []() {
            return "Hello World";
        };
        data dat("header", data{part});
        CHECK(tmpl.render(dat) == "Hello World");
    }

    SECTION("context") {
        mustache tmpl{"{{>header}}"};
        partial part{[]() {
            return "Hello {{name}}";
        }};
        data dat("header", data{part});
        dat["name"] = "Steve";
        CHECK(tmpl.render(dat) == "Hello Steve");
    }
    
    SECTION("nested") {
        mustache tmpl{"{{>header}}"};
        partial header{[]() {
            return "Hello {{name}} {{>footer}}";
        }};
        partial footer{[]() {
            return "Goodbye {{#names}}{{.}}|{{/names}}";
        }};
        data names{data::type::list};
        names.push_back("Jack");
        names.push_back("Jill");
        data dat("header", header);
        dat["name"] = "Steve";
        dat["footer"] = data{footer};
        dat["names"] = data{names};
        CHECK(tmpl.render(dat) == "Hello Steve Goodbye Jack|Jill|");
    }

    SECTION("dotted") {
        mustache tmpl{"{{>a.b}}"};
        partial a_b{[]() {
            return "test";
        }};
        data data("a.b", a_b);
        CHECK(tmpl.render(data) == "test");
    }
}

TEST_CASE("lambdas") {
    
    SECTION("basic") {
        mustache tmpl{"{{lambda}}"};
        data dat("lambda", data{lambda{[](const std::string&){
            return "Hello {{planet}}";
        }}});
        dat["planet"] = "world";
        CHECK(tmpl.render(dat) == "Hello world");
    }

    SECTION("basic_t") {
        mustache tmpl{"{{lambda}}"};
#if MUSTACHE_VS2013
        data dat("lambda", data{lambda_t{lambda_t::type1{[](const std::string&){
#else
        data dat("lambda", data{lambda_t{{[](const std::string&){
#endif
            return "Hello {{planet}}";
        }}}});
        dat["planet"] = "world";
        CHECK(tmpl.render(dat) == "Hello world");
    }

    SECTION("delimiters") {
        mustache tmpl{"{{= | | =}}Hello, (|&lambda|)!"};
        data dat("lambda", data{lambda{[](const std::string&){
            return "|planet| => {{planet}}";
        }}});
        dat["planet"] = "world";
        CHECK(tmpl.render(dat) == "Hello, (|planet| => world)!");
    }

    SECTION("nocaching") {
        mustache tmpl{"{{lambda}} == {{{lambda}}} == {{lambda}}"};
        int calls = 0;
        data dat("lambda", data{lambda{[&calls](const std::string&){
            ++calls;
            return std::to_string(calls);
        }}});
        CHECK(tmpl.render(dat) == "1 == 2 == 3");
    }

    SECTION("escape") {
        mustache tmpl{"<{{lambda}}{{{lambda}}}"};
        data dat("lambda", data{lambda{[](const std::string&){
            return ">";
        }}});
        CHECK(tmpl.render(dat) == "<&gt;>");
    }
    
    SECTION("section") {
        mustache tmpl{"<{{#lambda}}{{x}}{{/lambda}}>"};
        data dat("lambda", data{lambda{[](const std::string& text){
            return text == "{{x}}" ? "yes" : "no";
        }}});
        CHECK(tmpl.render(dat) == "<yes>");
    }

    SECTION("section_expansion") {
        mustache tmpl{"<{{#lambda}}-{{/lambda}}>"};
        data dat("lambda", data{lambda{[](const std::string& text){
            return text + "{{planet}}" + text;
        }}});
        dat["planet"] = "Earth";
        CHECK(tmpl.render(dat) == "<-Earth->");
    }

    SECTION("section_alternate_delimiters") {
        mustache tmpl{"{{= | | =}}<|#lambda|-|/lambda|>"};
        data dat("lambda", data{lambda{[](const std::string& text){
            return text + "{{planet}} => |planet|" + text;
        }}});
        dat["planet"] = "Earth";
        CHECK(tmpl.render(dat) == "<-{{planet}} => Earth->");
    }

    const lambda sectionLambda{[](const std::string& text){
        return "__" + text + "__";
    }};

    SECTION("section_multiple_calls") {
        mustache tmpl{"{{#lambda}}FILE{{/lambda}} != {{#lambda}}LINE{{/lambda}}"};
        data data("lambda", sectionLambda);
        CHECK(tmpl.render(data) == "__FILE__ != __LINE__");
    }

    SECTION("section_inverted") {
        mustache tmpl{"<{{^lambda}}{{static}}{{/lambda}}>"};
        data data("lambda", sectionLambda);
        data["static"] = "static";
        CHECK(tmpl.render(data) == "<>");
    }

}

TEST_CASE("dotted_names") {
    
    SECTION("basic") {
        mustache tmpl{"\"{{person.name}}\" == \"{{#person}}{{name}}{{/person}}\""};
        data person{"name", "Joe"};
        CHECK(tmpl.render({"person", person}) == "\"Joe\" == \"Joe\"");
    }

    SECTION("triple_mustache") {
        mustache tmpl{"\"{{{person.name}}}\" == \"{{#person}}{{name}}{{/person}}\""};
        data person{"name", "Joe"};
        CHECK(tmpl.render({"person", person}) == "\"Joe\" == \"Joe\"");
    }

    SECTION("ampersand") {
        mustache tmpl{"\"{{&person.name}}\" == \"{{#person}}{{&name}}{{/person}}\""};
        data person{"name", "Joe"};
        CHECK(tmpl.render({"person", person}) == "\"Joe\" == \"Joe\"");
    }

    SECTION("depth") {
        mustache tmpl{"\"{{a.b.c.d.e.name}}\" == \"Phil\""};
        data data{"a", {"b", {"c", {"d", {"e", {"name", "Phil"}}}}}};
        CHECK(tmpl.render(data) == "\"Phil\" == \"Phil\"");
    }

    SECTION("broken_chains1") {
        mustache tmpl{"\"{{a.b.c}}\" == \"\""};
        data data{"a", data::type::list};
        CHECK(tmpl.render(data) == "\"\" == \"\"");
    }

    SECTION("broken_chains2") {
        mustache tmpl{"\"{{a.b.c.name}}\" == \"\""};
        data data;
        data["a"] = {"b", data::type::list};
        data["c"] = {"name", "Jim"};
        CHECK(tmpl.render(data) == "\"\" == \"\"");
    }

    SECTION("depth2") {
        mustache tmpl{"\"{{#a}}{{b.c.d.e.name}}{{/a}}\" == \"Phil\""};
        data data;
        data["a"] = {"b", {"c", {"d", {"e", {"name", "Phil"}}}}};
        data["b"] = {"c", {"d", {"e", {"name", "Wrong"}}}};
        CHECK(tmpl.render(data) == "\"Phil\" == \"Phil\"");
    }

    SECTION("scope") {
        mustache tmpl{"\"{{#a}}{{b.name}}{{/a}}\" == \"Phil\""};
        data data;
        data["a"] = {"x", "y"};
        data["b"] = {"name", "Phil"};
        CHECK(tmpl.render(data) == "\"Phil\" == \"Phil\"");
    }
}

TEST_CASE("bustache_benchmark") {

    // https://github.com/jamboree/bustache/blob/master/test/benchmark.cpp
    int n = 0;
    object dat
    {
        {"header", "Colors"},
        {"items",
            list
            {
                object
                {
                    {"name", "red"},
                    {"first", true},
                    {"url", "#Red"}
                },
                object
                {
                    {"name", "green"},
                    {"link", true},
                    {"url", "#Green"}
                },
                object
                {
                    {"name", "blue"},
                    {"link", true},
                    {"url", "#Blue"}
                }
            }
        },
        {"empty", false},
        {"count", lambda{[&n](const std::string&) { return std::to_string(++n); }}},
        {"array", list{"1", "2", "3"}},
        {"a", object{{"b", object{{"c", true}}}}},
        {"comments",
            list
            {
                object
                {
                    {"name", "Joe"},
                    {"body", "<html> should be escaped"}
                },
                object
                {
                    {"name", "Sam"},
                    {"body", "{{mustache}} can be seen"}
                },
                object
                {
                    {"name", "New"},
                    {"body", "break\nup"}
                }
            }
        }
    };

}

TEST_CASE("lambda_render") {

    SECTION("auto-render") {
        mustache tmpl{"{{#wrapped}}{{name}} is awesome.{{/wrapped}}"};
        data data;
        data["name"] = "Willy";
        data["wrapped"] = lambda{[](const std::string& text) {
            CHECK(text == "{{name}} is awesome.");
            return "<b>" + text + "</b>";
        }};
        CHECK(tmpl.render(data) == "<b>Willy is awesome.</b>");
    }

    SECTION("no-render") {
        mustache tmpl{"{{#wrapped}}{{name}} is awesome.{{/wrapped}}"};
        data data;
        data["name"] = "Willy";
        data["wrapped"] = lambda2{[](const std::string& text, const renderer&) {
            CHECK(text == "{{name}} is awesome.");
            return "<b>" + text + "</b>";
        }};
        CHECK(tmpl.render(data) == "<b>{{name}} is awesome.</b>");
    }

    SECTION("no-render-lambda_t") {
        mustache tmpl{"{{#wrapped}}{{name}} is awesome.{{/wrapped}}"};
        data data;
        data["name"] = "Willy";
#if MUSTACHE_VS2013
        data["wrapped"] = lambda_t{lambda_t::type2{[](const std::string& text, const renderer&) {
#else
        data["wrapped"] = lambda_t{{[](const std::string& text, const renderer&) {
#endif
            CHECK(text == "{{name}} is awesome.");
            return "<b>" + text + "</b>";
        }}};
        CHECK(tmpl.render(data) == "<b>{{name}} is awesome.</b>");
    }

    SECTION("manual-render") {
        mustache tmpl{"{{#wrapped}}{{name}} is awesome.{{/wrapped}}"};
        data data;
        data["name"] = "Willy";
        data["wrapped"] = lambda2{[](const std::string& text, const renderer& render) {
            CHECK(text == "{{name}} is awesome.");
            const auto renderedText = render(text);
            CHECK(renderedText == "Willy is awesome.");
            return "<b>" + renderedText + "</b>";
        }};
        CHECK(tmpl.render(data) == "<b>Willy is awesome.</b>");
    }

    SECTION("manual-render-lambda_t") {
        mustache tmpl{"{{#wrapped}}{{name}} is awesome.{{/wrapped}}"};
        data data;
        data["name"] = "Willy";
#if MUSTACHE_VS2013
        data["wrapped"] = lambda_t{lambda_t::type2{[](const std::string& text, const renderer& render) {
#else
        data["wrapped"] = lambda_t{{[](const std::string& text, const renderer& render) {
#endif
            CHECK(text == "{{name}} is awesome.");
            const auto renderedText = render(text);
            CHECK(renderedText == "Willy is awesome.");
            return "<b>" + renderedText + "</b>";
        }}};
        CHECK(tmpl.render(data) == "<b>Willy is awesome.</b>");
    }

    SECTION("manual-render-append-tag") {
        // When using the render lambda, any text returned should not be itself rendered.
        mustache tmpl{"{{#wrapped}}{{name}} is awesome.{{/wrapped}}"};
        data data;
        data["name"] = "Willy";
        data["wrapped"] = lambda2{[](const std::string& text, const renderer& render) {
            CHECK(text == "{{name}} is awesome.");
            const auto renderedText = render(text);
            CHECK(renderedText == "Willy is awesome.");
            return "<b>" + renderedText + "</b>Hello {{name}}.";
        }};
        CHECK(tmpl.render(data) == "<b>Willy is awesome.</b>Hello {{name}}.");
    }

    SECTION("manual-render-error") {
        mustache tmpl{"{{#wrapped}}{{name}} is awesome.{{/wrapped}}"};
        data data;
        data["name"] = "Willy";
        data["wrapped"] = lambda2{[](const std::string& text, const renderer& render) -> mustache::string_type {
            CHECK(text == "{{name}} is awesome.");
            const auto renderedText = render("{{name is awesome");
            CHECK(renderedText == "");
            return {};
        }};
        CHECK(tmpl.render(data) == "");
        CHECK_FALSE(tmpl.is_valid());
        CHECK(tmpl.error_message() == "Unclosed tag at 0");
    }

    SECTION("lambda-render-variable") {
        mustache tmpl{"{{name}} is awesome."};
        data data;
        data["name"] = lambda2{[](const std::string&, const renderer&) -> mustache::string_type {
            return {};
        }};
        CHECK(tmpl.render(data) == "");
        CHECK_FALSE(tmpl.is_valid());
        CHECK(tmpl.error_message() == "Lambda with render argument is not allowed for regular variables");
    }

}

TEST_CASE("custom_escape") {

    SECTION("basic") {
        mustache tmpl("printf(\"Say {{password}} and enter\");{{&newline}}");
        tmpl.set_custom_escape([](const std::string& s) {
            std::string ret; ret.reserve(s.size());
            for (const auto ch: s) {
                switch (ch) {
                    case '\"':
                    case '\n':
                        ret.append({'\\', ch});
                        break;
                    default:
                        ret.append(1, ch);
                        break;
                }
            }
            return ret;
        });
        object data{ { "password", "\"friend\"" }, { "newline", "\n" } };
        CHECK(tmpl.render(data) == "printf(\"Say \\\"friend\\\" and enter\");\n");
    }

    SECTION("no_html_escape") {
        // make sure when using a custom escape that HTML is not escaped
        mustache tmpl("hello {{world}}");
        tmpl.set_custom_escape([](const std::string& s) {
            std::string ret; ret.reserve(s.size());
            for (const auto ch: s) {
                switch (ch) {
                    case '\"':
                    case '\n':
                        ret.append({'\\', ch});
                        break;
                    default:
                        ret.append(1, ch);
                        break;
                }
            }
            return ret;
        });
        object data{ { "world", "<world>" } };
        CHECK(tmpl.render(data) == "hello <world>");
    }

    SECTION("lambda") {
        mustache tmpl{"hello {{lambda}}"};
        data dat("lambda", data{lambda{[](const std::string&){
            return "\"friend\"";
        }}});
        tmpl.set_custom_escape([](const std::string& s) {
            std::string ret; ret.reserve(s.size());
            for (const auto ch: s) {
                switch (ch) {
                    case '\"':
                    case '\n':
                        ret.append({'\\', ch});
                        break;
                    default:
                        ret.append(1, ch);
                        break;
                }
            }
            return ret;
        });
        CHECK(tmpl.render(dat) == "hello \\\"friend\\\"");
    }

    SECTION("#lambda") {
        mustache tmpl{"hello {{#quote}}friend{{/quote}}"};
#if MUSTACHE_VS2013
        data dat1("quote", data{lambda_t{lambda_t::type2{[](const std::string& s, const renderer& r){
#else
        data dat1("quote", data{lambda_t{{[](const std::string& s, const renderer& r){
#endif
            return r("<\"" + s + "\">", true);
        }}}});
#if MUSTACHE_VS2013
        data dat2("quote", data{lambda_t{lambda_t::type2{[](const std::string& s, const renderer& r){
#else
        data dat2("quote", data{lambda_t{{[](const std::string& s, const renderer& r){
#endif
            return r("<\"" + s + "\">", false);
        }}}});
        tmpl.set_custom_escape([](const std::string& s) {
            std::string ret; ret.reserve(s.size());
            for (const auto ch: s) {
                switch (ch) {
                    case '\"':
                    case '\n':
                        ret.append({'\\', ch});
                        break;
                    default:
                        ret.append(1, ch);
                        break;
                }
            }
            return ret;
        });
        CHECK(tmpl.render(dat1) == "hello <\\\"friend\\\">");
        CHECK(tmpl.render(dat2) == "hello <\"friend\">");
    }

    SECTION("partial") {
        mustache tmpl{"hello {{>partial}}"};
        object dat({{"what", "\"friend\""}, {"partial", data{partial{[](){
            return "{{what}}";
        }}}}});
        tmpl.set_custom_escape([](const std::string& s) {
            std::string ret; ret.reserve(s.size());
            for (const auto ch: s) {
                switch (ch) {
                    case '\"':
                    case '\n':
                        ret.append({'\\', ch});
                        break;
                    default:
                        ret.append(1, ch);
                        break;
                }
            }
            return ret;
        });
        CHECK(tmpl.render(dat) == "hello \\\"friend\\\"");
    }

    SECTION("none") {
        mustache tmpl("hello {{what}}");
        mustache::escape_handler esc;
        tmpl.set_custom_escape(esc);
        object dat({ {"what", "\"friend\""} });
        CHECK_THROWS_AS(tmpl.render(dat), std::bad_function_call);
    }

}

template <typename string_type>
class my_context : public basic_context<string_type> {
public:
    my_context()
        : value_("Steve")
    {
    }

    virtual void push(const basic_data<string_type>* /*data*/) override {
    }

    virtual void pop() override {
    }

    virtual const basic_data<string_type>* get(const string_type& name) const override {
        if (name == "what") {
            return &value_;
        }
        return nullptr;
    }

    virtual const basic_data<string_type>* get_partial(const string_type& /*name*/) const override {
        return nullptr;
    }

private:
    basic_data<string_type> value_;
};

TEST_CASE("custom_context") {

    SECTION("basic") {
        my_context<mustache::string_type> ctx;
        mustache tmpl("Hello {{what}}");
        CHECK(tmpl.render(ctx) == "Hello Steve");
    }

    SECTION("empty") {
        my_context<mustache::string_type> ctx;
        mustache tmpl("Hello {{world}}");
        CHECK(tmpl.render(ctx) == "Hello ");
    }

}

template <typename string_type>
class file_partial_context : public context<string_type> {
public:
    file_partial_context(const basic_data<string_type>* data)
        : context<string_type>(data)
    {
    }

    virtual const basic_data<string_type>* get_partial(const string_type& name) const override {
        const auto cached = cached_files_.find(name);
        if (cached != cached_files_.end()) {
            return &cached->second;
        }
        string_type result;
        if (read_file(name, result)) {
            return &cached_files_.insert(std::make_pair(name, basic_data<string_type>(result))).first->second;
        }
        return nullptr;
    }

private:
    bool read_file(const string_type& name, string_type& file_contents) const {
        // read from file [name].mustache (fake the data for the test)
        if (name == "what") {
            file_contents = "World";
            return true;
        }
        return false;
    }

    mutable std::unordered_map<string_type, basic_data<string_type>> cached_files_;
};

TEST_CASE("file_partial_context") {

    data dat("punctuation", "!");
    file_partial_context<mustache::string_type> ctx{&dat};
    mustache tmpl("Hello {{>what}}{{punctuation}}");
    CHECK(tmpl.render(ctx) == "Hello World!");

}
