# include "BigUnsignedInt.hpp"

bool BigUnsignedInt::isOutStringHexMode = false;

BigUnsignedInt::BigUnsignedInt()
{
	dataSize = 1;
	data = new BaseData[dataSize];
	data[0] = 0;
	numLength = 0;

}
BigUnsignedInt::BigUnsignedInt(int num)
{
	if (num < 0)
		throw(std::underflow_error("Becomes negative number"));
	dataSize = 1;
	data = new BaseData[dataSize];
	data[0] = num;
	numLength = (num == 0) ? 0 : 1;
}
BigUnsignedInt::BigUnsignedInt(unsigned int num)
{
	dataSize = 1;
	data = new BaseData[dataSize];
	data[0] = num;
	numLength = (num == 0) ? 0 : 1;
}
BigUnsignedInt::BigUnsignedInt(long long num)
{
	if (num < 0)
		throw(std::underflow_error("Becomes negative number"));
	dataSize = 1;
	data = new BaseData[dataSize];
	data[0] = num;
	numLength = (num == 0) ? 0 : 1;
}
BigUnsignedInt::BigUnsignedInt(unsigned long long num)
{
	dataSize = 1;
	data = new BaseData[dataSize];
	data[0] = num;
	numLength = (num == 0) ? 0 : 1;
}

BigUnsignedInt::BigUnsignedInt(const BigUnsignedInt& num)
{
	dataSize = num.dataSize;
	data = new BaseData[dataSize];
	std::memcpy(data, num.data, sizeof(BaseData)* num.dataSize);
	numLength = num.numLength;
}
BigUnsignedInt::BigUnsignedInt(BigUnsignedInt && num)
{
	dataSize = num.dataSize;
	data = num.data;
	numLength = num.numLength;

	num.data = nullptr;
	num.dataSize = 0;
	num.numLength = 0;
}

BigUnsignedInt::~BigUnsignedInt()
{
	delete[] data;
}

BigUnsignedInt operator+(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	if (num1.numLength > num2.numLength) {
		BigUnsignedInt temp(num1);
		temp += num2;
		return temp;
	}
	else {
		BigUnsignedInt temp(num2);
		temp += num1;
		return temp;
	}
}
BigUnsignedInt operator-(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	BigUnsignedInt temp(num1);
	temp -= num2;
	return temp;
}
BigUnsignedInt operator*(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{//TODO
 //temporary code
	BigUnsignedInt ret;
	BigUnsignedInt temp = num1;
	unsigned int num2HighestBit = num2.floorLog2();
	for (unsigned int i = 0; i <= num2HighestBit; i++) {
		if (num2.getNthBit(i) != 0)
			ret += temp;
		temp <<= 1;
	}
	return ret;
}
BigUnsignedInt& BigUnsignedInt::operator+=(const BigUnsignedInt& num)
{
	BaseData MidResult;
	BaseData carry = 0;
	bool isCarry = false;

	if (num.numLength > dataSize)
		this->IncreaseDataSize(num.dataSize);

	for (unsigned int i = 0; i < num.numLength; i++) {
		BaseData& a = data[i];
		const BaseData& b = num.data[i];
		MidResult = a + b + carry;
		isCarry = isCarry ? (a >= MidResult) : (a > MidResult);
		carry = isCarry ? 1 : 0;
		a = MidResult;
	}

	for (unsigned int i = num.numLength; i < numLength; i++) {
		if (isCarry == false)
			break;
		BaseData& a = data[i];
		a++; // carry should be 1
		isCarry = (a == 0);//a==0 then another carry happens and a!=0 then carry = 0
		carry = isCarry ? 1 : 0;
	}

	numLength = std::max(num.numLength, numLength);
	if (isCarry) {
		numLength++;
		if (numLength > dataSize) {
			this->IncreaseDataSize(2 * dataSize);
		}
		data[numLength - 1] = 0x1;
	}
	return (*this);
}
BigUnsignedInt& BigUnsignedInt::operator-=(const BigUnsignedInt& num)
{
	bool isBorrowing = false;
	BaseData Borrowed = 0;
	if ((*this) < num) {
		throw std::underflow_error("Becomes negative number");
	}
	for (unsigned int i = 0; i < num.numLength; i++) {
		BaseData& a = data[i];
		const BaseData& b = num.data[i];
		Borrowed = isBorrowing ? 1 : 0; //check if borrowed at last loop or not
		isBorrowing = isBorrowing ? (a <= b) : (a < b); //is current borrowing from above
		a = (a - b) - Borrowed;
	}
	if (isBorrowing) {
		for (unsigned int i = num.numLength; i < numLength; i++) {
			BaseData& a = data[i];
			isBorrowing = (a == 0);
			a -= 1;
			if (!isBorrowing)
				break;
		}
	}
	if (data[numLength - 1] == 0) {
		for (int i = numLength - 2; i >= 0; i--) {
			if (data[i] != 0) {
				numLength = i + 1;
				break;
			}
		}
		if (data[numLength - 1] == 0) // this case each i at loop data[i]=0, so numLength didn't changed so num = 0
			numLength = 0;
	}
	return (*this);
}
BigUnsignedInt& BigUnsignedInt::operator*=(const BigUnsignedInt& num)
{
	(*this) = (*this) * num;
	return (*this);
}
BigUnsignedInt& BigUnsignedInt::operator++()
{
	bool carry = true;
	for (unsigned int i = 0; i < numLength; i++) {
		BaseData& a = data[i];
		a++;
		if (a != 0) {
			carry = false;
			break;
		}
	}
	if (carry) {
		numLength++;
		if (numLength > dataSize) {
			this->IncreaseDataSize(2 * dataSize);
		}
		data[numLength - 1] = 0x1;
	}
	return (*this);
}
BigUnsignedInt& BigUnsignedInt::operator--()
{
	bool borrowing = true;
	if (numLength == 0) {
		throw(std::underflow_error("Becomes negative number"));
		return (*this);
	}
	for (unsigned int i = 0; i < numLength; i++) {
		if (!borrowing)
			break;
		if (data[i] != 0)
			borrowing = false;
		data[i]--;
	}
	if (data[numLength - 1] == 0)
		numLength--;
	return (*this);
}
BigUnsignedInt BigUnsignedInt::operator++(int)
{
	BigUnsignedInt temp(*this);
	++(*this);
	return temp;
}
BigUnsignedInt BigUnsignedInt::operator--(int)
{
	BigUnsignedInt temp(*this);
	--(*this);
	return temp;
}

BigUnsignedInt& BigUnsignedInt::operator=(const BigUnsignedInt& num)
{
	if (this == &num)
		return (*this);

	delete[] data;
	data = new BaseData[num.dataSize];
	memcpy(data, num.data, sizeof(BaseData)*num.dataSize);
	dataSize = num.dataSize;
	numLength = num.numLength;
	return (*this);
}
BigUnsignedInt& BigUnsignedInt::operator=(BigUnsignedInt && num)
{
	if (this == &num)
		return (*this);

	delete[] data;
	data = num.data;
	dataSize = num.dataSize;
	numLength = num.numLength;

	num.data = nullptr;
	num.dataSize = 0;
	num.numLength = 0;
	return (*this);
}

BigUnsignedInt operator<<(const BigUnsignedInt& num, unsigned int k)
{
	BigUnsignedInt temp(num);
	temp <<= k;
	return temp;
}
BigUnsignedInt operator<<(const BigUnsignedInt& num, int k)
{
	BigUnsignedInt temp(num);
	temp <<= k;
	return temp;
}
BigUnsignedInt operator>>(const BigUnsignedInt& num, unsigned int k)
{
	BigUnsignedInt temp(num);
	temp >>= k;
	return temp;
}
BigUnsignedInt operator>>(const BigUnsignedInt& num, int k)
{
	BigUnsignedInt temp(num);
	temp >>= k;
	return temp;
}

BigUnsignedInt operator&(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	BigUnsignedInt ret;
	unsigned int min = std::min(num1.numLength, num2.numLength);
	ret.IncreaseDataSize(std::min(num1.dataSize, num2.dataSize));
	for (unsigned int i = 0; i < min; i++)
		ret.data[i] = num1.data[i] & num2.data[i];

	for (int i = min - 1; i >= 0; i--) {
		if (ret.data[i] != 0) {
			ret.numLength = i + 1;
			break;
		}
	}
	if (ret.data[ret.numLength - 1] == 0)
		ret.numLength = 0;
	return ret;
}
BigUnsignedInt operator|(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	if (num1.numLength < num2.numLength) {
		BigUnsignedInt num(num2);
		num |= num1;
		return num;
	}
	else {
		BigUnsignedInt num(num1);
		num |= num2;
		return num;
	}
}
BigUnsignedInt operator^(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	if (num1.numLength < num2.numLength) {
		BigUnsignedInt num(num2);
		num ^= num1;
		return num;
	}
	else {
		BigUnsignedInt num(num1);
		num ^= num2;
		return num;
	}
}
BigUnsignedInt& BigUnsignedInt::operator<<=(unsigned int k)
{
	unsigned int move = k / BaseDataLen;
	unsigned int a = k%BaseDataLen;
	unsigned int b = BaseDataLen - a;
	unsigned int newLen = ((this->floorLog2() + k) / BaseDataLen) + 1;

	if (numLength == 0 || k == 0)
		return (*this);


	if (dataSize < newLen) {
		unsigned int newDataSize = dataSize;
		while (newDataSize < newLen)
			newDataSize *= 2;
		this->IncreaseDataSize(newDataSize);
	}


	if (a == 0) {////if a==0 b = BaseDataLen, but num>>BaseDataLen == num , doesn't do anything. so, have to consider this case
		for (unsigned int i = numLength + move - 1; i >= move; i--)
			data[i] = data[i - move];
		for (unsigned int i = 0; i < move; i++)
			data[i] = 0;
	}
	else {
		if ((data[numLength - 1] >> b) != 0) { //if this value 0 then 
			data[numLength + move] = (data[numLength - 1] >> b);
		}
		data[numLength - 1 + move] = data[numLength - 1] << a;

		for (int i = numLength - 2; i >= 0; i--) {
			BaseData& buffer = data[i];
			data[i + move + 1] |= buffer >> b;
			data[i + move] = buffer << a;
		}
		for (unsigned int i = 0; i < move; i++)
			data[i] = 0;
	}
	numLength = newLen;
	return (*this);
}
BigUnsignedInt& BigUnsignedInt::operator<<=(int k)
{
	unsigned int num;
	if (k < 0) {
		num = -k;
		return ((*this) >>= num);
	}
	else {
		num = k;
		return ((*this) <<= num);
	}
}
BigUnsignedInt& BigUnsignedInt::operator>>=(unsigned int k)
{
	unsigned int move = k / BaseDataLen;
	unsigned int a = k%BaseDataLen;
	unsigned int b = BaseDataLen - a;
	unsigned int newLen = (this->floorLog2() >= k) ? (((this->floorLog2() - k) / BaseDataLen) + 1) : 0;

	if (numLength == 0 || k == 0) {
		return (*this);
	}

	if (a == 0) {////if a==0 b = BaseDataLen, but num<<BaseDataLen == num , doesn't do anything so, have to consider this case
		for (unsigned int i = move; i < numLength; i++)
			data[i - move] = data[i];
		for (unsigned int i = numLength - move; i < numLength; i++)
			data[i] = 0;
	}
	else {
		data[0] = (data[move] >> a);
		for (unsigned int i = move + 1; i < numLength; i++) {
			BaseData& buffer = data[i];
			data[i - move - 1] |= (buffer << b);
			data[i - move] = (buffer >> a);
		}
		for (unsigned int i = numLength - move; i < numLength; i++)
			data[i] = 0;
	}
	numLength = newLen;
	return (*this);
}
BigUnsignedInt& BigUnsignedInt::operator>>=(int k)
{
	unsigned int num;
	if (k < 0) {
		num = -k;
		return ((*this) <<= num);
	}
	else {
		num = k;
		return ((*this) >>= num);
	}
}
BigUnsignedInt& BigUnsignedInt::operator&=(const BigUnsignedInt& num)
{
	unsigned int min = std::min(numLength, num.numLength);
	for (unsigned int i = 0; i < min; i++)
		data[i] &= num.data[i];
	for (unsigned int i = min; i < numLength; i++)
		data[i] = 0;
	for (int i = min - 1; i >= 0; i--) {
		if (data[i] != 0) {
			numLength = i + 1;
			break;
		}
	}
	if (data[numLength - 1] == 0)
		numLength = 0;
	return (*this);
}
BigUnsignedInt& BigUnsignedInt::operator|=(const BigUnsignedInt& num)
{
	if (num.numLength > dataSize)
		this->IncreaseDataSize(num.dataSize);
	for (unsigned int i = 0; i < num.numLength; i++) {
		data[i] |= num.data[i];
	}
	numLength = std::max(numLength, num.numLength);
	return (*this);
}
BigUnsignedInt& BigUnsignedInt::operator^=(const BigUnsignedInt& num)
{
	if (num.numLength > dataSize)
		this->IncreaseDataSize(num.dataSize);
	for (unsigned int i = 0; i < num.numLength; i++) {
		data[i] ^= num.data[i];
	}
	for (int i = numLength - 1; i >= 0; i--) {
		if (data[i] != 0) {
			numLength = i + 1;
			break;
		}
	}
	if (data[numLength - 1] == 0)
		numLength = 0;
	return (*this);
}
bool operator==(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	if (num1.numLength != num2.numLength)
		return false;

	for (unsigned int i = 0; i < num1.numLength; i++) {
		if (num1.data[i] != num2.data[i])
			return false;
	}
	return true;
}
bool operator!=(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	return !(num1 == num2);
}

bool operator<(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	if (num1.numLength < num2.numLength)
		return true;
	if (num1.numLength > num2.numLength)
		return false;
	//when num1.numLength == num2.numLength
	for (int i = num1.numLength - 1; i >= 0; i--) {
		if (num1.data[i] < num2.data[i])
			return true;
		if (num1.data[i] > num2.data[i])
			return false;
	}
	return false;
}
bool operator<=(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	if (num1.numLength < num2.numLength)
		return true;
	if (num1.numLength > num2.numLength)
		return false;
	//when num1.numLength == num2.numLength
	for (int i = num1.numLength - 1; i >= 0; i--) {
		if (num1.data[i] < num2.data[i])
			return true;
		if (num1.data[i] > num2.data[i])
			return false;
	}
	return true;
}
bool operator>(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	return !(num1 <= num2);
}
bool operator>=(const BigUnsignedInt& num1, const BigUnsignedInt& num2)
{
	return !(num1 < num2);
}

bool BigUnsignedInt::operator!() const
{
	return (numLength == 0);
}
BigUnsignedInt::operator bool() const
{
	return (numLength != 0);
}

BigUnsignedInt::operator std::string() const
{
	if (isOutStringHexMode)
		return this->getHexStr();
	else
		return this->getDexStr();
}
std::string BigUnsignedInt::getHexStr() const
{
	const unsigned int NumOfCharInBaseData = 2 * sizeof(BaseData);
	char num2char[] = "0123456789ABCDEF";
	char * HexForm;
	unsigned int k = 2;//start from 2 (first "0x")
	unsigned int StartPos = 0;

	if (numLength == 0)
		return "0x0";

	HexForm = new char[NumOfCharInBaseData*numLength + 3];//+1 for NULL +2 for"0x"
	for (int i = this->numLength - 1; i >= 0; i--) {
		BaseData num = data[i];
		for (int j = NumOfCharInBaseData - 1; j >= 0; j--) {
			HexForm[k + j] = num2char[num & 0xF];
			num >>= 4;
		}
		k += NumOfCharInBaseData;
	}
	HexForm[k] = 0;

	for (unsigned int i = 2; i <= k; i++) {
		if (HexForm[i] != '0') {
			StartPos = i - 2;
			HexForm[StartPos] = '0';
			HexForm[StartPos + 1] = 'x';

			break;
		}
	}
	std::string result(HexForm + StartPos);

	delete[] HexForm;
	return result;
}
std::string BigUnsignedInt::getDexStr() const //double dabble algorithm
{
	const double log10_2 = std::log10(2);
	const unsigned int FloorLog2 = this->floorLog2();
	const unsigned int size = ((unsigned int)std::ceil((FloorLog2 + 1) * log10_2)) + 1;
	const int FragmentEndPos = ((FloorLog2 + 1) / 3) * 3;

	if (numLength == 0)
		return "0";

	int highestBit = size - 1;
	char * DexForm = new char[size + 1];//+1 is for NULL
	unsigned int firstNonZeroPos = 0;
	int carry = 0;
	int temp;


	//to use 3 bits in one loop think the remainder bits first
	temp = 0;
	for (int i = FloorLog2; i >= FragmentEndPos; i--) {
		temp <<= 1;
		temp |= (this->getNthBit(i)) ? 0x1 : 0x0;//remainder <3. so, just 0~2 bits and no carry.
	}
	DexForm[size - 1] = temp;


	for (int i = FragmentEndPos - 1; i >= 0; i -= 3) { // 3 bits in one loop
		carry = this->getNthBit(i) ? 0x1 : 0x0;
		carry <<= 1;
		carry |= this->getNthBit(i - 1) ? 0x1 : 0x0;
		carry <<= 1;
		carry |= this->getNthBit(i - 2) ? 0x1 : 0x0;
		for (int j = size - 1; j >= highestBit; j--) { //multiplying 8
			temp = DexForm[j];
			temp <<= 3;
			temp |= carry; // first carry < 8, temp<=9, so, (temp<<3) +  carry < 80. so, all next carry also satisfies carry < 8
			carry = temp / 10;
			temp %= 10;
			DexForm[j] = temp;
		}
		if (carry != 0)
			DexForm[--highestBit] = carry;//carry < 8
	}
	for (unsigned int i = highestBit; i < size; i++) {
		if (DexForm != 0) {
			firstNonZeroPos = i;
			break;
		}
	}
	for (unsigned int i = firstNonZeroPos; i < size; i++)
		DexForm[i] += '0';
	DexForm[size] = 0;

	std::string ret(DexForm + firstNonZeroPos);
	delete[] DexForm;
	return ret;
}
unsigned int BigUnsignedInt::floorLog2() const
{
	if (numLength == 0)
		return -1;
	unsigned int result = (numLength - 1)*BaseDataLen - 1;
	BaseData highestData = data[numLength - 1];
	while (highestData != 0) {
		result++;
		highestData >>= 1;
	}
	return result;
}
void BigUnsignedInt::stringConversionHexMod(bool sign)
{
	BigUnsignedInt::isOutStringHexMode = sign;
}

void BigUnsignedInt::IncreaseDataSize(unsigned int newSize)
{
	if (newSize <= dataSize)
		return;
	BaseData * newData;
	newData = new BaseData[newSize];

	std::memcpy(newData, data, sizeof(BaseData)*numLength);
	std::memset(newData + numLength, 0, sizeof(BaseData) * (newSize - numLength));
	delete[] data;
	data = newData;
	dataSize = newSize;
}

bool BigUnsignedInt::getNthBit(unsigned int n) const
{
	unsigned int ArrayPos = n / BaseDataLen;
	unsigned int DataPos = n % BaseDataLen;
	if (n > (numLength*BaseDataLen))
		return 0;
	return (data[ArrayPos] >> DataPos) & 0x1;
}