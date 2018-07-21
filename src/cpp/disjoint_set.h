#pragma once

#include "base.h"

class DisjointSet
{
protected:
	size_t n;
	vector<size_t> p;
	vector<unsigned> rank;
	vector<size_t> vsize;
	stack<size_t> ts;

public:
	DisjointSet(size_t _n)
	{
		Init(_n);
	}

	void Init(size_t _n)
	{
		n = _n;
		p.resize(n);
		for (size_t i = 0; i < n; ++i)
			p[i] = i;
		rank.resize(n);
		fill(rank.begin(), rank.end(), 0);
		vsize.resize(n);
		fill(vsize.begin(), vsize.end(), 1);
	}

	void Union(size_t i1, size_t i2)
	{
		UnionI(Find(i1), Find(i2));
	}

	size_t Find(size_t x)
	{
		size_t px = p[x];
		if (px == x)
			return px;
		size_t ppx = p[px];
		if (ppx == px)
			return px;
		do
		{
			ts.push(x);
			x = px;
			px = ppx;
			ppx = p[px];
		} while (px != ppx);
		while (!ts.empty())
		{
			x = ts.top();
			p[x] = ppx;
			ts.pop();
		}
		return ppx;
	}

	size_t GetSize(size_t x)
	{
		return vsize[Find(x)];
	}

protected:
	void UnionI(size_t i1, size_t i2)
	{
        if (i1 == i2)
            return;
		if (rank[i1] > rank[i2])
		{
			p[i2] = i1;
			vsize[i1] += vsize[i2];
		}
		else
		{
			p[i1] = i2;
			if (rank[i1] == rank[i2])
				++rank[i1];
			vsize[i2] += vsize[i1];
		}
	}
};
