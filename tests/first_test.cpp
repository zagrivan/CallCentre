#include <gtest/gtest.h>
#include <string>
#include "handle_req_resp.h"


TEST(ValidationUrlPhoneNumberTest, ValidPhoneNumberTest)
{
    const std::vector<std::string> valid_phone_numbers = {
            "/%20813%20555%200123",
            "/012%20344%200123",
            "/(813) 234-0211",
            "/(124)888-1002",
            "/88195778963",
            "/+78195778963",
            "/+7 (819) 577 8963",
            "/+7%20819%20577%208963",
            "/819%20577%208963",
    };

    for (const auto& num: valid_phone_numbers)
    {
        std::string prepared_number{};
        bool ans = call_c::isValidPhoneNumber(num, prepared_number);
        ASSERT_TRUE(ans);
    }
}

TEST(ValidationUrlPhoneNumberTest, InvalidPhoneNumberTest)
{
    const std::vector<std::string> valid_phone_numbers = {
            "/1234512312352135",
            "/123",
            "/123123",
            "/123%20567%2012",
            "/%20",
            "79157897878"
    };

    for (const auto& num: valid_phone_numbers)
    {
        std::string prepared_number{};
        bool ans = call_c::isValidPhoneNumber(num, prepared_number);
        ASSERT_FALSE(ans);
    }
}

TEST(ValidationUrlPhoneNumberTest, PreparedValidPhoneNumberTest)
{
    const std::map<std::string, std::string> valid_phone_numbers = {
            {"/%20813%20555%200123", "78135550123"},
            {"/012%20344%200123", "70123440123"},
            {"/+7%20819%20577%208963", "78195778963"},
            {"/819%20577%208963", "78195778963"},
            {"/(813) 234-0211", "78132340211"},
            {"/(124)888-1002", "71248881002"},
            {"/88195778963", "78195778963"},
            {"/+78195778963", "78195778963"},
            {"/+7 (819) 577 8963", "78195778963"}
    };

    for (const auto&[number, pure_val] : valid_phone_numbers)
    {
        std::string prepared_number{};
        bool ans = call_c::isValidPhoneNumber(number, prepared_number);
        ASSERT_EQ(prepared_number, pure_val);
    }
}

TEST(ValidationUrlPhoneNumberTest, PreparedInvalidPhoneNumberTest)
{
    const std::vector<std::string> valid_phone_numbers = {
            "/1234512312352135",
            "/123",
            "/123123",
            "/123%20567%2012",
            "/%20",
            "79157897878"
    };

    for (const auto& num: valid_phone_numbers)
    {
        std::string prepared_number{};
        bool ans = call_c::isValidPhoneNumber(num, prepared_number);
        ASSERT_EQ(prepared_number, std::string(""));
    }
}