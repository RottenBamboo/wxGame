#pragma once
#include <cmath>
#include <stdarg.h>
#include <initializer_list>
#define PI 3.1415926535898f
#define DOU_PI 3.1415926535898f * 2.0f
#define SET_ROW	1;
#define SET_COL	0;

namespace Mathmatic
{
	template <int count, typename T>
	struct Vector
	{
		Vector() {}

		Vector(const std::initializer_list<T> &t)
		{
			auto itr = t.begin();
			for (int itrct = 0; itrct != count; itrct++)
			{

				if (itr < t.end())
				{
					element[itrct] = *itr;
					itr++;
				}
				else
				{
					element[itrct] = 0;
				}
			}
		}

		Vector(T elem)
		{
			for (int itr = 0; itr != count; itr++)
			{
				element[itr] = elem;
			}
		}

		Vector(const Vector<count, T>& vec)
		{
			for (int itr = 0; itr != count; itr++)
			{
				element[itr] = vec.element[itr];
			}
		}

		Vector(int vcount, T* vec)
		{
			int itr = 0;
			for (int itrct = 0; itrct != count; itrct++)
			{
				if (itr < vcount)
				{
					element[itr] = vec[itr];
					itr++;
				}
				else
				{
					element[itr] = 0;
				}
			}
		}

		Vector<count, T>& operator=(const Vector<count, T>& vec2)
		{
			for (int itr = 0; itr != count; itr++)
			{
				element[itr] = vec2.element[itr];
			}
			return *this;
		}

		T element[count];
	};

	typedef Vector<2, float> Vector2FT;
	typedef Vector<3, float> Vector3FT;
	typedef Vector<4, float> Vector4FT;
	typedef Vector<4, float> Quaternion;

	template <template<int, typename> class TT = Vector, int count, typename T>
	TT<count, T> operator+(const TT<count, T>& vec1, const TT<count, T>& vec2)
	{
		TT<count, T> sum;
		for (int itr = 0; itr != count; itr++)
		{
			sum.element[itr] = vec1.element[itr] + vec2.element[itr];
		}
		return sum;
	}

	template <template<int, typename> class TT = Vector, int count, typename T>
	TT<count, T>& operator+=(TT<count, T>& vec1, const TT<count, T>& vec2)
	{
		for (int itr = 0; itr != count; itr++)
		{
			vec1.element[itr] = vec1.element[itr] + vec2.element[itr];
		}
		return vec1;
	}

	template <template<int, typename> class TT = Vector, int count, typename T>
	TT<count, T> operator-(const TT<count, T>& vec1, const TT<count, T>& vec2)
	{
		TT<count, T> sub;
		for (int itr = 0; itr != count; itr++)
		{
			sub.element[itr] = vec1.element[itr] - vec2.element[itr];
		}
		return sub;
	}

	template <template<int, typename> class TT = Vector, int count, typename T>
	TT<count, T>& operator-=(TT<count, T>& vec1, const TT<count, T>& vec2)
	{
		for (int itr = 0; itr != count; itr++)
		{
			vec1.element[itr] = vec1.element[itr] - vec2.element[itr];
		}
		return vec1;
	}

	template <template<int, typename> class TT = Vector, int count, typename T>
	T DotProduct(const TT<count, T>& vec1, const TT<count, T>& vec2)
	{
		T sum = 0;
		for (int itr = 0; itr != count; itr++)
		{
			sum += vec1.element[itr] * vec2.element[itr];
		}
		return sum;
	}

	template<template<int, typename> class TT = Vector, int count, typename T>
	TT<count, T> vector3CrossProduct(const TT<count, T>& vec1, const TT<count, T>& vec2)
	{
		TT<count, T> result;

		if (count < 3)
		{
			for (int itr = 0; itr != count; itr++)
			{
				result.element[itr] = vec1.element[itr];
			}
			return result;
		}

		result.element[0] = vec1.element[1] * vec2.element[2] - vec1.element[2] * vec2.element[1];
		result.element[1] = vec1.element[2] * vec2.element[0] - vec1.element[0] * vec2.element[2];
		result.element[2] = vec1.element[0] * vec2.element[1] - vec1.element[1] * vec2.element[0];

		for (int itr = 3; itr != count; itr++)
		{
			result.element[itr] = 0;
		}
		return result;
	}

	template <template<int, typename> class TT = Vector, int count, typename T>
	T vectorLength(const TT<count, T>& vec)
	{
		T result = 0;
		for (int itr = 0; itr != count; itr++)
		{
			result += vec.element[itr] * vec.element[itr];
		}

		return (T)sqrt(result);
	}

	template <template<int, typename> class TT = Vector, int count, typename T>
	TT<count, T> vectorNormalize(const TT<count, T>& vec)
	{
		TT<count, T> result;
		T vec_length = vectorLength(vec);
		for (int itr = 0; itr != count; itr++)
		{
			result.element[itr] = vec.element[itr] / vec_length;
		}
		return result;
	}

	template <int rows, int cols, typename T>
	struct Matrix
	{
		Matrix() {}

		Matrix(const std::initializer_list<T> &t)
		{
			Matrix(0);
			auto itrt = t.begin(); 
			
			for (int itrr = 0; itrr != rows; itrr++)
			{
				
				for (int itrc = 0; itrc != cols; itrc++)
				{
					if (itrt < t.end())
					{
						element[itrr][itrc] = *itrt;
						itrt++;
					}
					else
					{
						element[itrr][itrc] = 0;
					}
				}
			}
		}

		Matrix(T elem)
		{
			for (int itr1 = 0; itr1 != rows; itr1++)
			{
				for (int itr2 = 0; itr2 != cols; itr2++)
				{
					element[itr1][itr2] = elem;
				}
			}
		}

		Matrix(const Matrix<rows, cols, T>& mat)
		{
			for (int itr1 = 0; itr1 != rows; itr1++)
			{
				for (int itr2 = 0; itr2 != cols; itr2++)
				{
					element[itr1][itr2] = mat.element[itr1][itr2];
				}
			}
		}

		Matrix(int matcount, T* mat)
		{
			for (int itr1 = 0; itr1 != rows; itr1++)
			{
				for (int itr2 = 0; itr2 != cols; itr2++)
				{
					if (itr2 + itr1 * cols < matcount)
					{
						element[itr1][itr2] = mat[itr1 * cols + itr2];
					}
					else
					{
						element[itr1][itr2] = 0;
					}
				}
			}
		}

		Matrix<rows, cols, T>& operator=(const Matrix<rows, cols, T>& mat)
		{
			for (int itr1 = 0; itr1 != rows; itr1++)
			{
				for (int itr2 = 0; itr2 != cols; itr2++)
				{
					element[itr1][itr2] = mat.element[itr1][itr2];
				}
			}
			return *this;
		}

		Vector<rows, T> GetCol(int getcols) const
		{
			Vector<rows, T> veccols;
			for (int itr = 0; itr != rows; itr++)
			{
				veccols.element[itr] = element[itr][getcols];
			}
			return veccols;
		}

		Vector<cols, T> GetRow(int getrows) const
		{
			Vector<cols, T> vecrows;
			for (int itr = 0; itr != cols; itr++)
			{
				vecrows.element[itr] = element[getrows][itr];
			}
			return vecrows;
		}

		void SetCol(int setcol, Vector<rows, T> vec)
		{
			for (int itr = 0; itr != rows; itr++)
			{
				element[rows][setcol] = vec.element[itr];
			}
		}

		void SetRow(int setrow, Vector<cols, T> vec)
		{
			for (int itr = 0; itr != cols; itr++)
			{
				element[setrow][itr] = vec.element[itr];
			}
		}

		T element[rows][cols];
	};

	typedef Matrix<3, 3, float> Matrix3X3FT;
	typedef Matrix<4, 4, float> Matrix4X4FT;

	template<template<int, int, typename> class TT = Matrix, int rows, int cols, typename T>
	Matrix<rows, cols, T> MatrixMultiScalar(const Matrix<rows, cols, T>& mat, const T scalar)
	{
		Matrix<rows, cols, T> matResult;
		for (int itr1 = 0; itr1 != rows; itr1++)
		{
			for (int itr2 = 0; itr2 != cols; itr2++)
			{
				matResult.element[itr1][itr2] = mat.element[itr1][itr2] * scalar;
			}
		}

		return matResult;
	}

	template<template<int, int, typename> class TT = Matrix, int rows, int cols, typename T, int cols1>
	Matrix<rows, cols1, T> MatrixMultiMatrix(const Matrix<rows, cols, T>& mat1, const Matrix<cols, cols1, T>& mat2)
	{
		Matrix<rows, cols1, T> result;
		for (int itr1 = 0; itr1 != rows; itr1++)
		{
			for (int itr2 = 0; itr2 != cols1; itr2++)
			{
				result.element[itr1][itr2] = DotProduct(mat1.GetRow(itr1), mat2.GetCol(itr2));
			}
		}
		return result;
	}

	template<template<int, int, typename> class TT = Matrix, int rows, int cols, typename T>
	Vector<cols, T> MatrixMultiVector(const Vector<rows, T> vec, const Matrix<rows, cols, T>& mat)
	{
		Vector<cols, T> result;
		for (int itr = 0; itr != cols; itr++)
		{
			result.element[itr] = DotProduct(vec, mat.GetCol(itr));
		}
		return result;
	}

	template<template<int, int, typename> class TT = Matrix, int rows, int cols, typename T>
	Matrix<cols, rows, T> MatrixTranspose(const Matrix<rows, cols, T>& mat)
	{
		Matrix<rows, cols, T> result;
		for (int itr = 0; itr != cols; itr++)
		{
			Vector<rows, T> vec = mat.GetCol(itr);
			result.SetRow(itr, vec);
		}
		return result;
	}

	template<template<int, int, typename> class TT = Matrix, int idesize, typename T>
	void MatrixIdentity(Matrix<idesize, idesize, T>& mat)
	{
		for (int itr1 = 0; itr1 != idesize; itr1++)
		{
			for (int itr2 = 0; itr2 != idesize; itr2++)
			{
				if(itr1 == itr2)
				{
					mat.element[itr1][itr2] = 1;
				}
				else
				{
					mat.element[itr1][itr2] = 0;
				}
			}
		}
	}

	template<template<int, int, typename> class TT = Matrix, int rows, typename T>
	Matrix<rows - 1, rows - 1, T> MatrixMinor(const Matrix<rows, rows, T>& mat, int trow, int tcols)
	{
		Matrix<rows - 1, rows - 1, T> tempMat;
		for (int itr1 = 0, titr1 = 0; itr1 != rows; itr1++, titr1++)
		{
			if (trow == itr1)
			{
				titr1--;
				continue;
			}
			for (int itr2 = 0, titr2 = 0; itr2 != rows; itr2++, titr2++)
			{
				if (tcols == itr2)
				{
					titr2--;
					continue;
				}
				tempMat.element[titr1][titr2] = mat.element[itr1][itr2];
			}
		}
		return tempMat;
	}

	template<template<int, int, typename> class TT = Matrix, int rows, typename T>
	T MatrixDeterminant(const Matrix<rows, rows, T>& mat)
	{
		T result = 0;
		for (int itr = 0; itr != rows; itr++)
		{
			result += pow(-1, itr + 1 + 1) * mat.element[0][itr] * MatrixDeterminant(MatrixMinor(mat, 0, itr));
		}
		return result;
	}

	template<template<int, int, typename> class TT = Matrix, int rows = 2, int cols = 2, typename T>
	T MatrixDeterminant(const Matrix<2, 2, T>& mat)
	{
		T result;
		return result = mat.element[0][0] * mat.element[1][1] - mat.element[0][1] * mat.element[1][0];
	}

	template<template<int, int, typename> class TT = Matrix, int rows = 1, int cols = 1, typename T>
	T MatrixDeterminant(const Matrix<1, 1, T>& mat)
	{
		T result;
		return mat.element[0][0];
	}

	template<template<int, int, typename> class TT = Matrix, int rows = 2, int cols = 2, typename T>
	Matrix<rows, rows, T> MatrixAdjoint(const Matrix<rows, rows, T>& mat)
	{
		Matrix<rows, rows, T> temp = 0;
		for (int itr1 = 0; itr1 != rows; itr1++)
		{
			for (int itr2 = 0; itr2 != rows; itr2++)
				temp.element[itr1][itr2] = pow(-1, itr1 + 1 + itr2 + 1) * MatrixDeterminant(MatrixMinor(mat, itr1, itr2));
		}
		Matrix<rows, rows, T> result;
		result = MatrixTranspose(temp);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix, int rows, int cols, typename T>
	Matrix<rows, cols, T> MatrixInverse(const Matrix<rows, cols, T>& mat)
	{
		T matDeter = MatrixDeterminant(mat);
		Matrix<rows, cols, T> result(0);
		if (matDeter == 0)
		{
			return result;
		}
		result = MatrixMultiScalar(MatrixAdjoint(mat), 1 / matDeter);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix, int rows, int cols, typename T>
	Vector<cols, T> LinearTransform(const Vector<rows, T> vec, const Matrix<rows, cols, T>& mat)
	{
		Vector<cols,T> result = MatrixMultiVector(vec, mat);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix>
	Matrix4X4FT MatrixTranslation(const Vector3FT& vec3ft)
	{
		float matrixElem[] = { 1.0f,0.0f,0.0f,0.0f,
			0.0f,1.0f,0.0f,0.0f,
			0.0f,0.0f,1.0f,0.0f,
			vec3ft.element[0],vec3ft.element[1],vec3ft.element[2],1.0f,
		};

		Matrix4X4FT result(16, matrixElem);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix>
	Matrix4X4FT MatrixScale(const Vector3FT& vec3ft)
	{
		float matrixElem[] = { vec3ft.element[0], 0.0f, 0.0f, 0.0f,
			0.0f, vec3ft.element[1], 0.0f, 0.0f,
			0.0f, 0.0f, vec3ft.element[2], 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
		Matrix4X4FT result(16, matrixElem);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix>
	Matrix4X4FT MatrixRotationX(const float angle)
	{
		float cos = cosf(angle), sin = sinf(angle);
		float tempElem[] = { 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f,  cos, sin, 0.0f,
			0.0f, -sin,  cos, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f, };
		Matrix4X4FT result(16, tempElem);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix>
	Matrix4X4FT MatrixRotationY(const float angle)
	{
		float cos = cosf(angle), sin = sinf(angle);
		float tempElem[] = { cos,  0.0f, -sin, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			sin,  0.0f,  cos, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f, };
		Matrix4X4FT result(16, tempElem);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix>
	Matrix4X4FT MatrixRotationZ(const float angle)
	{
		float cos = cosf(angle), sin = sinf(angle);
		float tempElem[] = { cos,  sin, 0.0f, 0.0f,
			-sin,  cos, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f, };
		Matrix4X4FT result(16, tempElem);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix>
	Matrix4X4FT MatrixRotationAxis(const Vector4FT& axis, const float angle)
	{
		float cos = cosf(angle), sin = sinf(angle), x = axis.element[0], y = axis.element[1], z = axis.element[2];
		float tempElem[] = { cos + x * x * (1.0f - cos), x * y * (1.0f - cos) + z * sin,  x * z * (1.0f - cos) - y * sin, 0.0f,
			x * y * (1.0f - cos) - z * sin, cos + y * y * (1.0f - cos), y * z * (1.0f - cos) + x * sin, 0.0f,
			x * z * (1.0f - cos) + y * sin, y * z * (1.0f - cos) - x * sin, cos + z * z * (1.0f - cos), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f, };

		Matrix4X4FT result(16, tempElem);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix>
	Matrix4X4FT MatrixRotationQuaternion(Quaternion q)
	{
		float x = q.element[0], y = q.element[0], z = q.element[0], w = q.element[3];
		float tempElem[] = { 1.0f - 2.0f * y * y - 2.0f * z * z, 2.0f * x * y + 2.0f * w * z, 2.0f * x * z - 2.0f * w * y, 0.0f,
			2.0f * x * y - 2.0f * w * z, 1.0f - 2.0f * x * x - 2.0f * z * z, 2.0f * y * z + 2.0f * w * x, 0.0f,
			2.0f * x * z + 2.0f * w * y, 2.0f * y * z - 2.0f * y * z - 2.0f * w * x, 1.0f - 2.0f * x * x - 2.0f * y * y, 0.0f,
			0.0f,   0.0f,   0.0f,   1.0f };

		Matrix4X4FT result(16, tempElem);
		return result;
	}

	template<template<int, int, typename> class TT = Matrix>
	Matrix4X4FT BuildViewMatrix(const Vector3FT position, const Vector3FT lookAt, const Vector3FT up)
	{
		Vector3FT axisZ = vectorNormalize(lookAt - position);
		Vector3FT axisX = vectorNormalize(vector3CrossProduct(up, axisZ));
		Vector3FT axisY = vectorNormalize(vector3CrossProduct(axisZ, axisX));
		float translZ = -DotProduct(axisZ, position);// the position vector in the camera view matrix is the opposite direction of that in the world matrix.
		float translY = -DotProduct(axisY, position);
		float translX = -DotProduct(axisX, position);
		Matrix4X4FT viewMaitrx = {	axisX.element[0],axisX.element[1],axisX.element[2],0.0f,
									axisY.element[0],axisY.element[1],axisY.element[2],0.0f,
									axisZ.element[0],axisZ.element[1],axisZ.element[2],0.0f,
									translX,		translY,		  translZ,		   1.0f};
		return viewMaitrx;
	}

	template<template<int,int,typename> class TT = Matrix>
	Matrix4X4FT BuildPerspectiveMatrixForLH(const float angleAxisY, const float aspectRatio, const float nearPlane, const float FarPlane)
	{
		Matrix4X4FT perspectiveMatrix = {
		 1.0f / (aspectRatio * tanf(angleAxisY * 0.5f)), 0.0f, 0.0f, 0.0f ,
		 0.0f, 1.0f / tanf(angleAxisY * 0.5f), 0.0f, 0.0f ,
		 0.0f, 0.0f, FarPlane / (FarPlane - nearPlane), 1.0f ,
		 0.0f, 0.0f, (-nearPlane * FarPlane) / (FarPlane - nearPlane), 0.0f };
		return perspectiveMatrix;
	}
}
