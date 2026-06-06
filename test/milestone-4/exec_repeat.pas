program ExecRepeat;
var n: integer;
begin
    n := 0;
    repeat
        n := n + 1;
    until n >= 3;
    writeln(n);
end.
