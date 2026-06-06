program ArrayIndexBad;
var
  data: array['A'..'C'] of integer;
begin
  data['Z'] := 1;
end.
