SUBDIRS=src

define make_subdir
 @for subdir in $(SUBDIRS) ; do \
 ( cd $$subdir && make $1) \
 done;
endef

.PHONY:all
all:
	$(call make_subdir , all)

.PHONY:clean
clean:
	$(call make_subdir , clean) 
