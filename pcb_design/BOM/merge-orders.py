bom_files = [
    "actus",
    "burchellii",
    "dicio",
    "dodsonii",
    "perennis",
    "sylvatica"
]

header = '"Ref"	"Qnty"	"Value"	"Cmp name"	"Ref. Mouser"	"Ref. Farnell"	"Price"'
order_farnell = dict()
order_mouser = dict()
order_no_code = []

for fn in bom_files:
    header_found = False
    with open(fn + ".csv") as f:
        entries = []
        for line in f:
            if not header_found:
                if line.strip() == header:
                    header_found = True
                    continue
                else:
                    continue
            entries.append(line.strip().split("\t"))

        if len(entries[-1]) < len(entries[0]):
            entries = entries[:-1]

        entries = [[i.replace('"', "") for i in e] for e in entries]

        for e in entries:
            if len(e[4]) < 3:
                if e[5] == "":
                    e.append(fn)
                    order_no_code.append(e)
                if e[5] in order_farnell:
                    order_farnell[e[5]] += int(e[1], 10)
                else:
                    order_farnell[e[5]] = int(e[1], 10)
            else:
                if e[4] in order_mouser:
                    order_mouser[e[4]] += int(e[1], 10)
                else:
                    order_mouser[e[4]] = int(e[1], 10)

                
with open("farnell.csv", 'w') as f:
    order_codes = sorted(order_farnell.keys()) 
    f.write("Order code, pieces\n")
    for code in order_codes:
        f.write("%s, %d\n" % (code, 3*order_farnell[code]))

with open("mouser.csv", 'w') as f:
    order_codes = sorted(order_mouser.keys()) 
    f.write("Order code, pieces\n")
    for code in order_codes:
        f.write("%s, %d\n" % (code, 3*order_mouser[code]))
    
for i in order_no_code:
    print(i)