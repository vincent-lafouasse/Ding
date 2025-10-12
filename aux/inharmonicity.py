kLs = [
    4.73004,
    7.8532,
    10.99561,
    14.137164,
    17.27876,
    20.52035,
]

ratios = [kL / kLs[0] for kL in kLs]
precision = 1000.0
ratios = [int(r * precision) / precision for r in ratios]

print(ratios)
