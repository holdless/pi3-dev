import sys
import os.path
import csv

if __name__ == "__main__":

    if len(sys.argv) != 2:
        print "usage: create_csv <base_path>"
        sys.exit(1)

    BASE_PATH=sys.argv[1]
    SEPARATOR=";"

    # prepare to write csv file
    f = open('blend_face_db.csv', 'wb')
    w = csv.writer(f)
    
    label = 0
    for dirname, dirnames, filenames in os.walk(BASE_PATH):
        for subdirname in dirnames:
            subject_path = os.path.join(dirname, subdirname)
            for filename in os.listdir(subject_path):
                if not filename.startswith('.'):
                    abs_path = "%s/%s" % (subject_path, filename)
                    print "%s%s%d" % (abs_path, SEPARATOR, label)
                    w.writerow([abs_path + SEPARATOR + str(label)])
            label = label + 1

    f.close()
