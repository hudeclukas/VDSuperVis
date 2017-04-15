inline QSet<int> parseRange(QString inputText)
{
	QSet<int> values;
	auto ranges = inputText.split(",");
	for (auto range : ranges)
	{
		auto numbers = range.split("-", QString::SkipEmptyParts);
		if (numbers.size() == 1)
		{
			values.insert(numbers[0].toInt());
		}
		if (numbers.size() == 2) 
		{
			int a = numbers[0].toInt();
			int b = numbers[1].toInt();
			if (b < a)
			{
				a += b;
				b = a - b;
				a -= b;
			}
			for (auto i = a; i <= b; i++)
			{
				values.insert(i);
			}
		}
	}
	return values;
}
