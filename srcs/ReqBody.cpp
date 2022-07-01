# include "ReqBody.hpp"
#include "Exceptions.hpp"

/**========================================================================
* @                           Constructors
*========================================================================**/

/**========================================================================
* *                            operators
*========================================================================**/

	ReqBody&	ReqBody::operator=( const ReqBody& src )
	{
		if (this != &src)
		{
			content = src.content;
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/
	void	ReqBody::clearChunk()
	{
		chunk.state = 0;
		chunk.cur = 0;
		chunk.newChunkStart = 0;
		chunk.data.clear();
		chunk.size = -1;
		chunk.total = 0;
		chunk.trailingData.clear();
	}

	void	ReqBody::appendChunk(const string& newdata)
	{
		chunk.data.append(newdata);
	}

	void	ReqBody::setChunk(const string& newdata)
	{
		chunk.data = newdata;
	}

	bool	ReqBody::canGoAhead(const string& s, string::size_type cur, string::size_type i)
	{
		if (cur + i < s.length())
			return true;
		return false;
	}

	void	ReqBody::printState(int state)
	{
		cout << "state is " ;
		switch (state) {
		case 0: cout << "SizeStart" << endl; break;
		case 1: cout << "ReadingSize" << endl; break;
		case 2: cout << "ReadingExt" << endl; break;
		case 3: cout << "SizeAlmostDone" << endl; break;
		case 4: cout << "SizeDone" << endl; break;
		case 5: cout << "ReadingData" << endl; break;
		case 6: cout << "DataAlmostDone" << endl; break;
		case 7: cout << "DataDone" << endl; break;
		case 8: cout << "AllDone" << endl; break;
		default:;
		}
	}
	void	ReqBody::decodingChunk(ssize_t bodyLimit)
	{
		long long i = 0;
		enum t {
		SizeStart,		// 0
		ReadingSize,	// 1
		ReadingExt,		// 2
		SizeAlmostDone,	// 3
		SizeDone,		// 4
		ReadingData,	// 5
		DataAlmostDone,	// 6
		DataDone,		// 7
		AllDone			// 8
		} state;


		for(;;)
		{
			/* get prev state. */
			state = (t)chunk.state;
			switch (state)
			{
			case DataDone:			// a chunk done. go to next chunk.
			{
				if (canGoAhead(chunk.data, chunk.cur,1))
				{
					chunk.cur++;
					chunk.newChunkStart = chunk.cur;
				}
				else
					goto _readmore;
			}

			case SizeStart:
			{
				if (chunk.data.empty() )
					goto _readmore;
				if (ishexnumber(chunk.data[chunk.cur]))
					goto _ReadingSize;
				else
					goto _invalid;

			}
			case ReadingSize:
				goto _ReadingSize;

			case SizeAlmostDone:
				goto _SizeAlmostDone;
			case SizeDone:
				goto _SizeDone;
			case ReadingData:
				goto _ReadingData;

			case DataAlmostDone:
				goto _DataAlmostDone;

			default:
				break;
			}


	_ReadingSize:
			state = ReadingSize;
			/*
				stop at
				1. last valid (maybe hex) char
				2. end char of data
			*/
			while (ishexnumber(chunk.data[chunk.cur]))
			{
				if (canGoAhead(chunk.data, chunk.cur, 1) &&
					ishexnumber(chunk.data[chunk.cur + 1]))
					chunk.cur++;
				else
					break;
			}

			if (canGoAhead(chunk.data, chunk.cur,1))
				chunk.cur++;
			else
				goto _readmore;

			if (chunk.data[chunk.cur] == '\r')
				state = SizeAlmostDone;
			else
				goto _invalid;

	_SizeAlmostDone:
			if (canGoAhead(chunk.data, chunk.cur, 1))
				chunk.cur++;
			else
				goto _readmore;


			if (chunk.data[chunk.cur] == '\n')
			{
				state = SizeDone;
				chunk.size = toHexNum<long long>(chunk.data.substr(chunk.newChunkStart,chunk.cur-1));
			}
			else
				goto _invalid;

	_SizeDone:
			chunk.total += chunk.size;

			if (bodyLimit && (bodyLimit < chunk.total))		//NOTE: 어디서부터 데이터를 버려야 하나?
				goto _limitExceeded;

			if (canGoAhead(chunk.data, chunk.cur,1))
				chunk.cur++;
			else
			{
				goto _readmore;
			}

	_ReadingData:
			state = ReadingData;
			/*
				1. data > chunk.size 	=> 	(i == chunk.size), break by for condition.
				2. data <= chunk.size	=>	(i != chunk.size), break by ! canGoAhead
			*/
			for (i = 0; i < chunk.size; i++)
			{
				if (canGoAhead(chunk.data, chunk.cur + i, 1))
					continue;
				else
					break;
			}

			if (i == chunk.size)
				chunk.cur += chunk.size;
			else
				goto _readmore;

			/*
				checking after chunk.size is safe,
				because only case 1 (data > chunk.size) can reach here.
			*/
			if (chunk.data[chunk.cur] == '\r')
				state = DataAlmostDone;
			else
				goto _invalid;


	_DataAlmostDone:
			if (canGoAhead(chunk.data, chunk.cur, 1))
				chunk.cur++;
			else
				goto _readmore;

			if (chunk.data[chunk.cur] == '\n')
			{
				if (chunk.size)
				{
					state = DataDone;
					goto _appendData;
				}
				else
				{
					state = AllDone;
					goto _AllDone;
				}
			}
			else
				goto _invalid;

	_appendData:
			chunk.state=state;
			this->append(chunk.data.substr(chunk.cur - chunk.size - 1,
											chunk.size));
		}

_readmore:
		chunk.state=state;
		// printState(chunk.state);
		throw readMore();

_invalid:
		chunk.state=state;
		// printState(chunk.state);
		clearChunk();
		throw invalidChunk();
_limitExceeded:
		chunk.state=state;
		// printState(chunk.state);
		clearChunk();
		throw limitExeeded();
_AllDone:
		chunk.state=state;
		if (chunk.state == AllDone)
		{
			if (canGoAhead(chunk.data, chunk.cur, 1))
				chunk.trailingData = chunk.data.substr(chunk.cur + 1);
			else
				chunk.trailingData = "";
		}
		return ;
	}

	void	ReqBody::clear() { content.clear(), clearChunk(); /*IText::clear();*/ }
