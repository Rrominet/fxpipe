#include "./Price.h"

json Price::serialize() const
{
    json _r;	
    _r["price"] = _price;
    _r["quantity"] = _quantity;
    _r["unit"] = _unit;
    return _r;
}

void Price::deserialize(const json& data)
{
    if (data.contains("price"))	
        _price = data["price"];
    if (data.contains("quantity"))	
        _quantity = data["quantity"];
    if (data.contains("unit"))	
        _unit = data["unit"];
}

// Overload the == operator
bool Price::operator==(const Price& color) const
{
    return (_price == color.price() && _quantity == color.quantity() && _unit == color.unit());
}
// Overload the != operator using operator==
bool Price::operator!=(const Price& color) const
{
    return !(*this == color);
}
