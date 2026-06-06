program ExecFuncCall;
var r: integer;
function Add(a, b: integer): integer;
begin
    Add := a + b;
end;
begin
    r := Add(3, 4);
    writeln(r);
end.
