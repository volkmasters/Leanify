#include "tar.h"


size_t Tar::Leanify(size_t size_leanified /*= 0*/)
{
    char *p_read = fp;
    fp -= size_leanified;
    char *p_write = fp;
    level++;
    int checksum = 0;

    do
    {
        checksum = CalcChecksum((unsigned char *)p_read);
        // 256 means the record is all 0
        if (checksum == 256)
        {
            break;
        }
        if (size_leanified)
        {
            memmove(p_write, p_read, 512);
        }
        p_read += 512;
        if (checksum != GetOctalNum(p_write + 148))
        {
            std::cout << "Checksum does not match!" << std::endl;
            p_write += 512;
            continue;
        }
        char type = *(p_write + 156);
        size_t original_size = GetOctalNum(p_write + 124);
        // align to 512
        size_t size_aligned = ((original_size - 1) & ~0x1FF) + 0x200;
        if (original_size)
        {
            if (type == 0 || type == '0')
            {
                // normal file
                if (is_verbose)
                {
                    for (int i = 0; i < level; i++)
                    {
                        std::cout << "-> ";
                    }
                    std::cout << p_write << std::endl;
                }
                size_t new_size = LeanifyFile(p_read, original_size, size_leanified);
                if (new_size < original_size)
                {
                    // write new size
                    sprintf(p_write + 124, "%011o", new_size);

                    // update checksum
                    sprintf(p_write + 148, "%06o", CalcChecksum((unsigned char *)p_write));
                    p_write[155] = ' ';

                    // align to 512
                    size_t new_size_aligned = ((new_size - 1) & ~0x1FF) + 0x200;

                    // make sure the rest space is all 0
                    memset(p_write + new_size + 512, 0, new_size_aligned - new_size);

                    p_write += new_size_aligned;
                    if (new_size_aligned != size_aligned)
                    {
                        size_leanified += size_aligned - new_size_aligned;
                    }
                }
                else
                {
                    // make sure the rest space is all 0
                    memset(p_write + new_size + 512, 0, size_aligned - new_size);
                    p_write += size_aligned;
                }
            }
            else
            {
                // other type, just move it
                memmove(p_write + 512, p_read, size_aligned);
                p_write += size_aligned;
            }
            p_read += size_aligned;
        }
        p_write += 512;

    } while (p_write < fp + size);

    // write 2 more zero-filled records
    memset(p_write, 0, 1024);
    size  = p_write + 1024 - fp;
    return size;
}


int Tar::CalcChecksum(unsigned char *header) const
{
    // checksum bytes are taken to be spaces
    // ' ' = 32, 32x8 = 256
    int s = 256;
    for (int i = 0; i < 148; i++)
    {
        s += header[i];
    }
    for (int i = 156; i < 512; i++)
    {
        s += header[i];
    }
    return s;
}



size_t Tar::GetOctalNum(char *p) const
{
    int n;
    sscanf(p, "%o", &n);
    return n;
}
