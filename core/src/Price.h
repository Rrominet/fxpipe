#pragma once
#include"str.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Price
{
    public : 
        Price() = default;
        Price(float price, float quantity, std::string unit) : _price(price), _quantity(quantity), _unit(unit) {}
        ~Price() = default;

        json serialize() const;
        void deserialize(const json& data);

        // Overload the == operator
        bool operator==(const Price& color) const;
        // Overload the != operator using operator==
        bool operator!=(const Price& color) const;

    private : 
        float _price = 0.0;//bp cgs
        float _quantity = 0.0;//bp cgs
        std::string _unit;//bp cgs

    public : 
#include "./Price_gen.h"
};
