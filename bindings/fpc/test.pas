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
  test_array: array of array of UInt8;
  ix: Integer;

  source_data, serialized_data: TByteDynArray;
  fwbt: TFWBT;
  err: TFWBTError;
begin
  SetLength(test_array, 4);
  test_array[0] := [1, 3, 1, 2];
  test_array[1] := [1, 2, 3, 4, 5, 6, 7, 8];
  test_array[2] := [1, 3, 3, 7];
  test_array[3] := [1, 2, 3, 4, 5, 6, 7, 8];

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

  { Test Modification }
  err := FwbtSetValue(fwbt, test_array[0], test_array[1], False);
  if err <> FWBT_OK then
  begin
    writeln('setvalue 1 error: ', err);
    halt;
  end;

  writeln('added value 1');
  writeln('value 1 at index ', FwbtFindValue(fwbt, [1, 3, 1, 2]));

  err := FwbtSetValue(fwbt, test_array[2], test_array[3], False);
  if err <> FWBT_OK then
  begin
    writeln('setvalue 2 error: ', err);
    halt;
  end;

  writeln('added value 2');
  writeln('value 2 at index ', FwbtFindValue(fwbt, [1, 3, 3, 7]));

  err := FwbtRemoveValue(fwbt, [1, 3, 1, 2]);
  if err <> FWBT_OK then
  begin
    writeln('removevalue error: ', err);
    halt;
  end;

  writeln('removed value 1');

  err := FwbtSerialize(fwbt, serialized_data);
  if err <> FWBT_OK then
  begin
    writeln('serialization error: ', err);
    halt;
  end;

  writeln('Serialized Size: ', Length(serialized_data));
  if CompareByteDynArray(source_data, serialized_data) then
  begin
    writeln('Serialized data matches input data!');
    halt;
  end;

  writeln('modifications persist');
end.
