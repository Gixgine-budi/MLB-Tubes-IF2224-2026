program ExecArray;
var data: array[1..3] of integer;
begin
    data[1] := 10;
    data[2] := 20;
    data[3] := 30;
    writeln(data[2]);
end.
