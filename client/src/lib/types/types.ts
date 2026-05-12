// export interface Drives: string[]

export interface dir{
    name: string,
    src: string,
    can_see: string | string[],
    can_upload: string | string[],
    can_delete: string | string[],
    can_download: string | string[],
}

export interface file{
    name: string,
    src: string,
    can_see: string | string[],
    can_download: string | string[],
}

export interface vd{
    name: string,
    dirs: dir[],
    files: file[],
    can_see: string | string[],
    can_download: string | string[],
}

