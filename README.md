# Console application based on Shapes which displays use of Listeners on DataReaders and DataWriters

Skeleton created via
 
```bash
rtiddsgen -language C++11 -platform x64Linux4gcc7.3.0 -example x64Linux4gcc7.3.0 -create makefiles -create typefiles -d c++11 shapes.idl
```

The Waitset was removed from the subscriber and replaced with a listener class derived from NoOpDataReaderListener which handles 
on_data_available override
