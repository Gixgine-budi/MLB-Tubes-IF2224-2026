program ExecNestedScope;
var x: integer;
procedure Inner;
var x: integer;
begin
    x := 2;
    writeln(x);
end;
begin
    x := 1;
    Inner();
    writeln(x);
end.
