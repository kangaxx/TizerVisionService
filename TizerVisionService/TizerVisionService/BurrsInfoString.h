#pragma once
//用于存储序列化的毛刺数据信息并存储到双向列表里
// 该类需要和serialization_c11库联合使用
//add by gxx 20210508

#include "../../../hds/serialization_c11.h"

class BurrsInfoString
{
private:
    char buffer[INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE];
public:
    BurrsInfoString(char* value) { this->setBuffer(value); }
    BurrsInfoString(const BurrsInfoString& right) { this->setBuffer(right.getBuffer()); }
    char* getBuffer() const { return (char*)buffer; }
    void setBuffer(char* value) { memcpy_s(buffer, INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE, value, INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE); }
};

