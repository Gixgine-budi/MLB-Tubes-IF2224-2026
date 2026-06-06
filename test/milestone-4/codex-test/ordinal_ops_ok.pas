program OrdinalOpsOk;
type
  Digit == 0..5;
  Color == (red, green, blue);
  Early == red..green;
var
  i: Digit;
  e: Early;
begin
  i := 2;
  e := red;
  if 'B' > 'A' then writeln(i + 3);
  if e <= green then writeln(9);
end.
