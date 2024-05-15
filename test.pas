program test;

uses libfwbt, Types;

function LoadBytesFromFile(const path: String): TByteDynArray;
var
  _file: File of Byte;
  buf: Byte;
  wix: Integer;
begin
  Assign(_file, path);
  
  ReSet(_file);
  SetLength(LoadBytesFromFile, FileSize(_file));

  wix := 0;
  while not eof(_file) do
  begin
    read(_file, buf);
    LoadBytesFromFile[wix] := buf;
    inc(wix);
  end;

  Close(_file);
end;

function CompareByteDynArray(constref array1, array2: TByteDynArray): Boolean;
var
  ix: Integer;
begin
  CompareByteDynArray := False;

  if Length(array1) <> Length(array2) then
    exit;

  for ix := 0 to Length(array1) - 1 do
  begin
    if array1[ix] <> array2[ix] then
      exit;
  end;

  CompareByteDynArray := True;
end;

var
  source_data, serialized_data: TByteDynArray;
  fwbt: TFWBT;
  err: TFWBTError;
begin
  source_data := LoadBytesFromFile('test.fwbt');
  if Length(source_data) = 0 then
    halt;

  writeln('Raw Size   : ', Length(source_data));

  { Test Parsing }
  err := FwbtParseBytes(fwbt, source_data);
  if err <> FWBT_OK then
  begin
    writeln('parsing error: ', err);
    halt;
  end;

  writeln('Key Width  : ', fwbt.header.key_width);
  writeln('Value Width: ', fwbt.header.value_width);
  writeln('Entry Count: ', fwbt.header.entry_count);

  { Test Serialization }
  err := FwbtSerialize(fwbt, serialized_data);
  if err <> FWBT_OK then
  begin
    writeln('serialization error: ', err);
    halt;
  end;
  
  writeln('Serialized Size: ', Length(serialized_data));
  if not CompareByteDynArray(source_data, serialized_data) then
  begin
    writeln('Serialized data does not match input data!');
    halt;
  end;
end.
