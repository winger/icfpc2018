#include "command.h"

void Command::Encode(vector<uint8_t>& v) const
{
    if (type == Halt)
        v.push_back(0b11111111);
    else if (type == Wait)
        v.push_back(0b11111110);
    else if (type == Flip)
        v.push_back(0b11111101);
    else if (type == SMove)
    {
        uint16_t t = EncodeLCD(cd1);
        v.push_back(((t >> 8) << 4) | 0b00000100);
        v.push_back(t);
    }
    else if (type == LMove)
    {
        uint16_t t1 = EncodeSCD(cd1);
        uint16_t t2 = EncodeSCD(cd2);
        v.push_back(((t1 >> 8) << 4) | ((t2 >> 8) << 6) | 0b00001100);
        v.push_back(t1 | (t2 << 4));
    }
    else if (type == FusionP)
    {
        uint8_t t = EncodeNCD(cd1);
        v.push_back((t << 3) | 0b00000111);
    }
    else if (type == FusionS)
    {
        uint8_t t = EncodeNCD(cd1);
        v.push_back((t << 3) | 0b00000110);
    }
    else if (type == Fission)
    {
        uint8_t t = EncodeNCD(cd1);
        v.push_back((t << 3) | 0b00000101);
        v.push_back(m);
    }
    else if (type == Fill)
    {
        uint8_t t = EncodeNCD(cd1);
        v.push_back((t << 3) | 0b00000011);
    }
    else
        assert(false);
}

void Command::Decode(const vector<uint8_t>& v, size_t& pos)
{
    uint8_t u1 = v[pos++];
    if (u1 == 0b11111111)
        type = Halt;
    else if (u1 == 0b11111110)
        type = Wait;
    else if (u1 == 0b11111101)
        type = Flip;
    else if ((u1 & 0b00001111) == 0b00000100)
    {
        type = SMove;
        uint16_t u2 = v[pos++];
        uint16_t t = ((uint16_t(u1) >> 4) << 8) | u2;
        cd1 = DecodeLCD(t);
    }
    else if ((u1 & 0b00001111) == 0b00001100)
    {
        type = LMove;
        uint16_t u2 = v[pos++];
        uint16_t t1 = (((uint16_t(u1) >> 4) & 3) << 8) | (u2 & 15);
        uint16_t t2 = (((uint16_t(u1) >> 6) & 3) << 8) | ((u2 >> 4) & 15);
        cd1 = DecodeSCD(t1);
        cd2 = DecodeSCD(t2);
    }
    else if ((u1 & 0b00000111) == 0b00000111)
    {
        type = FusionP;
        cd1 = DecodeNCD(u1 >> 3);
    }
    else if ((u1 & 0b00000111) == 0b00000110)
    {
        type = FusionS;
        cd1 = DecodeNCD(u1 >> 3);
    }
    else if ((u1 & 0b00000111) == 0b00000101)
    {
        type = Fission;
        cd1 = DecodeNCD(u1 >> 3);
        m = v[pos++];
    }
    else if ((u1 & 0b00000111) == 0b00000011)
    {
        type = Fill;
        cd1 = DecodeNCD(u1 >> 3);
    }
    else
        assert(false);
}
