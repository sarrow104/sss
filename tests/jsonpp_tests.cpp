
// void test_query(sss::string_view s)
// {
//     // std::vector<std::function<sss::string_view(sss::string_view)> > query_list = {
//     //     std::bind(sss::json::query_object_name, std::placeholders::_1, "hello"),
//     //     std::bind(sss::json::query_array_index, std::placeholders::_1, 2)
//     // };
// 
//     // sss::string_view value = s;
//     // for (auto& f : query_list) {
//     //     value = f(value);
//     // }
//     // COLOG_ERROR(SSS_VALUE_MSG(value));
//     // std::cout << sss::raw_string(value) << std::endl;
// 
//     // std::cout << query<int>(s, 1, "hello", 0) << std::endl;
//     std::cout << query<int>(s, sss::string_view("hello"), 0) << std::endl;
//     std::cout << query<int>(s, sss::string_view("hello"), 1) << std::endl;
//     std::cout << query<int>(s, sss::string_view("hello"), 2) << std::endl;
//     std::cout << query<Null_t>(s, sss::string_view("hello"), 3) << std::endl;
// }

