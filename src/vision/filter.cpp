#include <arrayfire.h>
using namespace af;

class SudokuFilter{
public:
	SudokuFilter();
	virtual ~SudokuFilter();

	array filter(const array& in);

private:
	array gray(const array& in);
	array blur(const array& in, int window_width, int window_height, int sigma);
	array threshold(const array& in, float thresholdVal);

};