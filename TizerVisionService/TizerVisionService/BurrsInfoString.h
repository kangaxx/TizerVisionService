#pragma once
//���ڴ洢���л���ë��������Ϣ���洢��˫���б���
// ������Ҫ��serialization_c11������ʹ��
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

