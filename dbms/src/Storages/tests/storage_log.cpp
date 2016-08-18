#include <iostream>

#include <DB/IO/WriteBufferFromOStream.h>
#include <DB/Storages/StorageLog.h>
#include <DB/DataStreams/TabSeparatedRowOutputStream.h>
#include <DB/DataStreams/LimitBlockInputStream.h>
#include <DB/DataStreams/BlockOutputStreamFromRowOutputStream.h>
#include <DB/DataStreams/copyData.h>
#include <DB/DataTypes/DataTypesNumberFixed.h>
#include <DB/Columns/ColumnsNumber.h>
#include <DB/Interpreters/Context.h>


int main(int argc, char ** argv)
try
{
	using namespace DB;

	const size_t rows = 10000000;

	/// создаём таблицу с парой столбцов

	NamesAndTypesListPtr names_and_types = std::make_shared<NamesAndTypesList>();
	names_and_types->push_back(NameAndTypePair("a", std::make_shared<DataTypeUInt64>()));
	names_and_types->push_back(NameAndTypePair("b", std::make_shared<DataTypeUInt8>()));

	StoragePtr table = StorageLog::create("./", "test", names_and_types);

	/// пишем в неё
	{
		Block block;

		ColumnWithTypeAndName column1;
		column1.name = "a";
		column1.type = table->getDataTypeByName("a");
		column1.column = column1.type->createColumn();
		ColumnUInt64::Container_t & vec1 = typeid_cast<ColumnUInt64&>(*column1.column).getData();

		vec1.resize(rows);
		for (size_t i = 0; i < rows; ++i)
			vec1[i] = i;

		block.insert(column1);

		ColumnWithTypeAndName column2;
		column2.name = "b";
		column2.type = table->getDataTypeByName("b");
		column2.column = column2.type->createColumn();
		ColumnUInt8::Container_t & vec2 = typeid_cast<ColumnUInt8&>(*column2.column).getData();

		vec2.resize(rows);
		for (size_t i = 0; i < rows; ++i)
			vec2[i] = i * 2;

		block.insert(column2);

		BlockOutputStreamPtr out = table->write({}, {});
		out->write(block);
	}

	/// читаем из неё
	{
		Names column_names;
		column_names.push_back("a");
		column_names.push_back("b");

		QueryProcessingStage::Enum stage;

		BlockInputStreamPtr in = table->read(column_names, 0, Context{}, Settings(), stage)[0];

		Block sample;
		{
			ColumnWithTypeAndName col;
			col.type = std::make_shared<DataTypeUInt64>();
			sample.insert(std::move(col));
		}
		{
			ColumnWithTypeAndName col;
			col.type = std::make_shared<DataTypeUInt8>();
			sample.insert(std::move(col));
		}

		WriteBufferFromOStream out_buf(std::cout);

		LimitBlockInputStream in_limit(in, 10, 0);
		RowOutputStreamPtr output_ = std::make_shared<TabSeparatedRowOutputStream>(out_buf, sample);
		BlockOutputStreamFromRowOutputStream output(output_);

		copyData(in_limit, output);
	}

	return 0;
}
catch (const DB::Exception & e)
{
	std::cerr << e.what() << ", " << e.displayText() << std::endl;
	return 1;
}
