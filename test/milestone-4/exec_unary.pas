program ExecUnary;
var x: integer;
    flag: boolean;
begin
    x := 5;
    x := -x;
    writeln(x);
    flag := not false;
    if flag then
        writeln(1)
    else
        writeln(0);
end.
