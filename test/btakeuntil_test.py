import os
import random
import shell
import string
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import lists, composite, integers, randoms, floats, text, sampled_from
from test_util import run, clone_source, compile_buffer_sizes

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([12, 17, 64, 128, 256, 1024, 1024 * 1024 * 5] + [random.randint(8, 1024) for _ in range(10)])))
else:
    buffers = [128]

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean', stream=True)
    compile_buffer_sizes('csv', buffers)
    compile_buffer_sizes('bsv', buffers)
    compile_buffer_sizes('bsort', buffers)
    compile_buffer_sizes('btakeuntil', buffers)
    shell.run('make bsv csv bsort btakeuntil', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    r = draw(randoms())
    buffer = draw(sampled_from(buffers))
    num_text_columns = draw(integers(min_value=1, max_value=4))
    text_column = text(string.ascii_lowercase, min_size=1, max_size=8)
    text_line = lists(text_column, min_size=num_text_columns, max_size=num_text_columns)
    lines = draw(lists(text_line, min_size=1))
    first_column_values = [line[0] for line in lines]
    threshold = draw(floats(min_value=0, max_value=1))
    for line in lines:
        if line and r.random() > threshold:
            line[0] = r.choice(first_column_values)
    csv = '\n'.join([','.join(l)[:buffer // 4] for l in lines if l]).strip() + '\n'
    value = r.choice(first_column_values)
    return value, csv, buffer

def expected(value, csv):
    res = []
    lines = csv.splitlines()
    lines = [line.split(',') for line in lines]
    lines = sorted(lines)
    for cols in lines:
        if cols:
            if cols[0] >= value:
                break
            res.append(','.join(str(c) for c in cols))
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    value, csv, buffer = args
    result = expected(value, csv)
    assert set(result.splitlines()) == set(run(csv, f'bsv.{buffer} | bsort.{buffer} | btakeuntil.{buffer} "{value}" | csv.{buffer}').splitlines()) # set because sort is not stable and is only for first column values

words = [
    "Abelson", "Aberdeen", "Allison", "Amsterdam", "Apollos", "Arabian",
    "Assad", "Austerlitz", "Bactria", "Baldwin", "Belinda", "Bethe", "Blondel",
    "Bobbitt", "Boone", "Bowery", "Browne", "Candy", "Carmella", "Cheever",
    "Chicano", "Christa", "Clyde", "Conakry", "Cotopaxi", "Dalai", "Damian",
    "Davidson", "Deana", "Dobro", "Dona", "Doritos", "Drew", "Eggo", "Elmer",
    "Eunice", "Everett", "Fauntleroy", "Fortaleza", "Frenchwoman", "Freudian",
    "Galatea", "Grenoble", "Gwendoline", "Hals", "Hastings", "Head", "Hilda",
    "Hoff", "Hohenzollern", "Hosea", "Internet", "Iranian", "Irene",
    "Israelis", "Jacobin", "Jansenist", "Jewishness", "Jorge", "Joy", "Judaeo",
    "Kaye", "Knuths", "Laval", "Leanna", "Lebanese", "Leroy", "Lieberman",
    "Louisville", "Loyang", "Loyola", "Lubavitcher", "Luke", "Luxembourger",
    "Macao", "Madeleine", "Maghreb", "Magus", "Maidenform", "Malabo",
    "Marissa", "Matthews", "Mauriac", "Mauritius", "Mauro", "Milne",
    "Mississippian", "Muscat", "NEH", "NFC", "Natalie", "Nellie", "Norman",
    "Novokuznetsk", "Olaf", "Ono", "PO", "Pittsburgh", "Presbyterianism",
    "Procrustean", "Proust", "Pugh", "Quixotism", "Rapunzel", "Rochester",
    "Rodrigo", "Schnabel", "Selectric", "Shavuot", "TARP", "Terrell", "Tony",
    "Topeka", "Tunisia", "Turner", "Ulysses", "Utah", "Valarie", "Veracruz",
    "Volta", "WTO", "Wallenstein", "Waring", "Woolf", "YMHA", "Yunnan",
    "Zedekiah", "Zoroastrians", "Zubeneschamali", "abhorred", "abnormal",
    "aborning", "abracadabra", "abscissa", "academic", "accelerating",
    "acculturates", "acquaintanceship", "acquiescing", "acupressure", "add",
    "addicts", "addling", "adjoins", "adulterants", "aerialists", "aerodromes",
    "affirmatives", "aftershaves", "afterthought", "aggravate", "airguns",
    "alewives", "alighting", "allies", "alpines", "alumnus", "ambidextrously",
    "ambled", "andirons", "antedating", "antics", "anxieties", "aphorisms",
    "appeasement", "appraisal", "arbitrating", "architect", "architectonics",
    "armband", "arsed", "asininity", "assessments", "assizes", "astigmatism",
    "astringency", "astronomers", "attains", "aunt", "auricular",
    "authentications", "autocratic", "averaging", "awhile", "babying",
    "backaches", "backstopped", "baker", "balancing", "balustrade",
    "bandmaster", "banning", "baptismal", "bareheaded", "beauts", "bedeviling",
    "beholders", "being", "belling", "bellyache", "bemoan", "beseecher",
    "besieged", "bespatters", "bethinking", "biggie", "biker", "biologist",
    "bipartisanship", "bivouac", "blackened", "bldg", "blistery",
    "bloodthirsty", "bludgeons", "boating", "bobbies", "bodacious", "bodies",
    "boniest", "bootlace", "bossing", "bouncily", "bowdlerizes", "bowlegs",
    "braes", "braggart", "brakes", "branching", "brazer", "breastfeed",
    "breather", "briefcase", "briefcases", "briefed", "brilliantly",
    "brotherhood", "broths", "browbeaten", "bucks", "builtin", "bully",
    "bunch", "burgomaster", "bursitis", "bursting", "bushes", "buyouts",
    "caber", "cafetieres", "caiman", "callus", "cankered", "cannibalizes",
    "canvasbacks", "capstan", "cardamom", "caregiver", "carouse",
    "catastrophe", "catatonic", "cavils", "cellulose", "certificated",
    "chambermaid", "channelizing", "chapeaus", "chappy", "charms",
    "chastisement", "chummed", "churchwoman", "ciceroni", "circuitry",
    "circumlocutory", "circumstantially", "classically", "clientele", "clomps",
    "clunkiest", "codger", "cods", "cogitated", "collaborator", "columbines",
    "comings", "commissions", "commutation", "competitively", "concealable",
    "conclusiveness", "confidentially", "confiscating", "conjurers",
    "conscientious", "continuing", "contractually", "converging", "convey",
    "cooling", "copulates", "cornrow", "corrugating", "cosigners",
    "cosmogonies", "cosmologists", "cosplay", "counseled", "countertenors",
    "crackle", "craving", "crayoning", "crazes", "creams", "credibly",
    "crookneck", "crowds", "cudgelings", "culminations", "cures", "curling",
    "curvaceous", "cusp", "cutaways", "cutting", "dandier", "dangerous",
    "darn", "dauntless", "dc", "deactivation", "dearest", "deceased",
    "decisiveness", "decomposes", "decrement", "defenestrations",
    "dehydrators", "delete", "dementia", "demonstratively", "depilatory",
    "deployment", "depolarizes", "deposed", "desensitized", "desiccates",
    "detergent", "devote", "diaphragms", "digesting", "dilate", "disastrously",
    "disencumber", "disfavored", "divides", "dobs", "docility", "dogfights",
    "doggedness", "domestic", "domesticate", "dongs", "dots", "doughnuts",
    "downing", "downspout", "dreamers", "dredging", "dribblers", "dubbed",
    "dun", "duration", "earpiece", "easing", "effectively", "effendis",
    "eggbeaters", "electable", "elimination", "elusively", "enabler",
    "enchantingly", "enciphered", "enciphering", "enervation", "enormity",
    "entangle", "enthralls", "enthuses", "entrap", "environment", "epigram",
    "epigraphy", "equalize", "equalizing", "equidistantly", "equipment",
    "equivocated", "erroneous", "escalloped", "esplanades", "eulogistic",
    "evacuate", "evils", "examiner", "expropriator", "extremest", "extrinsic",
    "falsified", "fancied", "fastbacks", "fathomless", "featherweights",
    "feds", "feminines", "fermium", "ferule", "fictitious", "filer",
    "filigrees", "firescreen", "fixing", "flatbread", "fleeting", "fleshlier",
    "flexing", "flickering", "flusher", "foretasted", "forevermore",
    "forwarding", "forwardness", "fossilize", "freeloads", "fretfulness",
    "frogman", "fuchsias", "fulfillment", "furthering", "gadgets",
    "galumphing", "galumphs", "game", "gamuts", "garroted", "gaze", "gazumped",
    "genes", "gentlefolk", "geodes", "geographers", "ghosting", "gimbals",
    "gloatingly", "goldbrickers", "goslings", "gouaches", "gracefulness",
    "grafts", "granddaughter", "grandiloquence", "granges", "grater", "grebes",
    "gregariousness", "grenadiers", "groping", "grosgrain", "grovelled",
    "guardrooms", "guesstimated", "gussied", "gusting", "gymnasts", "gyps",
    "haggard", "harmlessly", "harrow", "haste", "hatreds", "headstones",
    "headword", "heartland", "heavy", "hellholes", "helpfully", "hindmost",
    "hoer", "holdover", "holes", "homewreckers", "homicides", "honeylocust",
    "horrendously", "hospitalization", "hostels", "however", "howls",
    "huddling", "hues", "human", "humanized", "humbler", "hushing",
    "hybridization", "hymnbooks", "hypoglycemic", "imbibers", "immunoglobulin",
    "impertinence", "impolite", "impolitic", "impressionists", "impulsively",
    "inappreciably", "incarnate", "incomprehensibly", "indebted", "indicted",
    "indissolubility", "inducted", "indulge", "inglenooks", "inheriting",
    "inhibitions", "inmate", "inner", "innocently", "innocuously",
    "inquisitors", "insecticide", "inserts", "inspiration", "institutional",
    "intent", "intermarry", "intriguer", "inveighs", "invertebrates",
    "inveterate", "inwards", "ironies", "isle", "iterated", "jobbing", "jolts",
    "journalese", "junker", "junking", "kale", "king", "kinkiness",
    "kissogram", "kluged", "laborious", "ladyships", "lancing", "lapped",
    "lapwings", "larcenist", "lawns", "laxity", "leafage", "led", "lenses",
    "lexers", "linebackers", "linoleum", "lint", "liquidizes", "livid",
    "locate", "lofty", "loggerheads", "lummoxes", "lunching", "macaronis",
    "magnificently", "maids", "mainstreamed", "maledictions", "mallow",
    "mangled", "manhandling", "mannequins", "mantis", "markka", "meagerly",
    "meanness", "meas", "meddlers", "mendicants", "microprocessor", "migrant",
    "mikados", "militarists", "miming", "miner", "miscommunications",
    "miscount", "misinterpret", "mistaken", "mistreat", "ml", "modernize",
    "modishly", "molars", "monaural", "moneymaker", "moneys", "monosyllabic",
    "moralizers", "more", "mountains", "mounted", "mowers", "mussier",
    "naturals", "necromancy", "neglecting", "nettle", "neutralizing",
    "newscasters", "nibbles", "nincompoop", "nonfictional", "noninflationary",
    "noninvasive", "nonthreatening", "nonunion", "novices", "null",
    "numerologist", "nutshells", "oak", "oath", "objurgate", "objurgations",
    "occlusion", "odd", "officeholder", "oiliest", "onside", "onsite", "oomph",
    "oozier", "opticians", "optimizes", "orgy", "osteoporosis", "outfield",
    "outlaws", "outlooks", "outstayed", "overate", "overcasts", "overcautious",
    "overdressing", "overfeeds", "overindulgence", "overlie", "overproduced",
    "overran", "pager", "paradox", "parches", "paroles", "partnered", "pasha",
    "patisseries", "patriarchs", "patronage", "peculator", "peculiarly",
    "peduncle", "peepbo", "pensively", "perfectas", "peripherals",
    "peritoneal", "perm", "persevering", "persisting", "persuasions",
    "petitioners", "pharmacology", "phasing", "phenol", "phenotype",
    "photocopying", "photostat", "physiognomies", "picks", "picnicking", "pie",
    "pilchards", "pinheads", "pitching", "plagues", "planned", "plantain",
    "playgroup", "pleasures", "plonk", "plunders", "poi", "pollinated",
    "popularize", "population", "postilion", "pother", "potpies", "potter",
    "pounces", "poundage", "praying", "precursor", "precursory", "presorting",
    "pressures", "prestige", "prevaricating", "previewers", "prickle",
    "private", "probabilistic", "procedural", "procurers", "prodigious",
    "progeny", "proliferates", "prolongation", "proselytizes", "protective",
    "protoplasmic", "provender", "provocative", "pseudonym", "pshaws",
    "psychological", "public", "puffier", "putsch", "quacked",
    "quadruplication", "quads", "quandary", "queered", "questioning",
    "quicksilver", "quoted", "radiate", "razor", "reappear", "reassigned",
    "reburied", "receptively", "recessives", "recipe", "reclassification",
    "recluses", "reconciliation", "recurrently", "redden", "redrafts",
    "redskin", "reduce", "reemphasizes", "reenacting", "refinement",
    "reforging", "reformations", "refurbished", "regard", "regenerate",
    "regretted", "rejoices", "relaxing", "remarriages", "rematch", "reminders",
    "renovation", "rephrasing", "repleted", "reprehensibly", "res", "reside",
    "responsively", "restated", "restoratives", "resultants", "retaught",
    "retype", "revenue", "rhombuses", "rigmarole", "rigor", "rinsed",
    "rissole", "roaster", "robocalls", "rogering", "roles", "rooming",
    "rosemary", "rotter", "ruggeder", "russet", "safflower", "sanctimony",
    "sandblaster", "sassafras", "sayings", "scapegraces", "scatology",
    "schnapps", "scintillating", "scourges", "scrams", "scrappiest",
    "screenshots", "scuffle", "sculleries", "seethes", "selloff",
    "sensualists", "sentimentalize", "sentimentalizing", "sexually", "sh",
    "shading", "shadowing", "shakiness", "shamelessness", "shareholdings",
    "shatters", "shearer", "shepherdess", "shits", "shorebird", "shutoff",
    "sibylline", "sierra", "silencing", "sitarists", "skillet", "slappers",
    "sleeps", "sleeved", "sliminess", "smack", "societies", "society", "sofas",
    "softened", "solemnly", "soliciting", "solitude", "soloists", "sols",
    "south", "sparring", "spewed", "sphinxes", "spiderweb", "spindliest",
    "spiritedly", "splays", "spuds", "stables", "staggeringly",
    "staphylococci", "starchy", "starved", "starves", "statehood", "statutory",
    "stets", "stigmatized", "stilettos", "stomacher", "strafe", "stratagems",
    "strategies", "stressing", "stringed", "stripling", "strongly",
    "subconsciously", "subcontracting", "subcontractors", "subdomains",
    "subliminal", "subroutines", "subverting", "succubi", "suffocated",
    "supermarket", "surged", "surlier", "surreys", "sussing", "swaddles",
    "swaddling", "switching", "syncopation", "tachyon", "tactfully",
    "tactlessness", "tailbacks", "tamales", "teaser", "tellingly", "temped",
    "tenoned", "tenterhooks", "tepee", "tethered", "theosophic", "therapeutic",
    "thermodynamic", "thickos", "this", "tho", "thoroughbreds", "ticktacktoe",
    "tinctured", "tintinnabulation", "titillatingly", "toastier",
    "tonsillectomy", "touchdown", "town", "towrope", "traction",
    "transferable", "transgenders", "transgenic", "trifle", "tundras",
    "twenty", "typing", "ulcerate", "unbosoms", "unbuttoning", "underclassman",
    "undertake", "underutilized", "undesired", "unfrock", "ungainlier",
    "unhesitating", "unknowable", "unloveliest", "unmanageable",
    "unrecognized", "unreliability", "unsportsmanlike", "uppercase",
    "utilizes", "vaccinations", "vamoose", "vapidity", "vaults", "veep",
    "venturesomeness", "vertex", "vest", "vestibule", "voyeur", "vulgarian",
    "wakes", "wangling", "washcloths", "wassailed", "watchful", "waterfalls",
    "weekender", "wheedling", "wherein", "whopping", "wildlife", "wilier",
    "windowpanes", "wingnuts", "wisest", "wogs", "woodiest", "woodlands",
    "woodworking", "woulds", "wrinklier", "wrongly", "yammered", "yest",
    "zebus", "zeitgeists", "zings",
]
